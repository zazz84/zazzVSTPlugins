/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <mmintrin.h>

//==============================================================================
const ModernRotarySlider::ParameterDescription HalfwaveSaturatorAudioProcessor::m_parametersDescritpion[] =
{
	{ "Drive",
	  " %",
	  "Drive" },
	
	{ "Offset",
	" dB",
	"Offset" },

	{ "Low Cut",
	  " Hz",
	  "Low Cut" },

	{ "Volume",
      " dB",
      "Volume" }
};

//==============================================================================
HalfwaveSaturatorAudioProcessor::HalfwaveSaturatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	for (int i = 0; i < Parameters::COUNT; i++)
    {
		m_parameters[i] = apvts.getRawParameterValue(m_parametersDescritpion[i].paramName);
    }
}

HalfwaveSaturatorAudioProcessor::~HalfwaveSaturatorAudioProcessor()
{
}

//==============================================================================
const juce::String HalfwaveSaturatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HalfwaveSaturatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HalfwaveSaturatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HalfwaveSaturatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HalfwaveSaturatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HalfwaveSaturatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HalfwaveSaturatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HalfwaveSaturatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HalfwaveSaturatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void HalfwaveSaturatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HalfwaveSaturatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_lowCutFilter[channel].init(sr);
	}

	// Initialize 4x oversampling, no latency compensation
	oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
		getTotalNumOutputChannels(),					// number of channels
		4,												// oversampling factor: 2^4 = 16x
		juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
		true);											// use interpolation for better quality
		
	oversampler->reset();
	oversampler->initProcessing(samplesPerBlock);

	setLatencySamples((int)oversampler->getLatencyInSamples());
}

void HalfwaveSaturatorAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HalfwaveSaturatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void HalfwaveSaturatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all params
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Samples loop constants
	const auto gain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);
	auto wetRaw = 0.01f * parametersValues[Parameters::Drive];								// Rescale to [0, 1]
	wetRaw *= wetRaw;																		// Power of 2
	wetRaw *= 0.6f;																		    // Limit to wet maximum of 75%
	const auto biasGainCompensation = juce::Decibels::decibelsToGain((2.0f / 3.0f) * parametersValues[Parameters::Offset]);
	const auto driveGainCompensation = juce::Decibels::decibelsToGain(3.0f * wetRaw);
	const auto wet = driveGainCompensation * biasGainCompensation * gain * wetRaw;
	const auto dry = gain * (1.0f - wetRaw);
	//const auto offsetGain = parametersValues[Parameters::Offset] < 0.0f ? juce::Decibels::decibelsToGain(parametersValues[Parameters::Offset]) - 1.0f : 0.5f * juce::Decibels::decibelsToGain(parametersValues[Parameters::Offset]) - 0.5f;
	const auto offsetGain = 0.5f * juce::Decibels::decibelsToGain(parametersValues[Parameters::Offset]) - 0.5f;

	// --- Upsample ---
	juce::dsp::AudioBlock<float> block(buffer);
	auto oversampledBlock = oversampler->processSamplesUp(block);

	// Now oversampledBlock has oversampled audio; adjust sample count
	const int overSamples = oversampledBlock.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = oversampledBlock.getChannelPointer(channel);

#if JUCE_USE_SSE_INTRINSICS
		int overSample = 0;

		// Broadcast constants once
		const __m128 vLo = _mm_set1_ps(offsetGain);
		const __m128 vDry = _mm_set1_ps(dry);
		const __m128 vWet = _mm_set1_ps(wet);

		// Process 4 samples per iteration
		for (; overSample <= overSamples - 4; overSample += 4)
		{
			// Load
			auto* firstSample = channelBuffer + overSample;
			__m128 in = _mm_loadu_ps(firstSample);

			// Half wave
			__m128 clipped = _mm_max_ps(in, vLo);

			// Dry / wet mix
			__m128 out = _mm_add_ps(_mm_mul_ps(vDry, in), _mm_mul_ps(vWet, clipped));

			// Store
			_mm_storeu_ps(firstSample, out);
		}

		// Tail processing (remaining samples)
		for (; overSample < overSamples; ++overSample)
		{
			float& in = channelBuffer[overSample];
			const float clipped = in < offsetGain ? offsetGain : in;
			in = dry * in + wet * clipped;
		}
#else
		for (int overSample = 0; overSample < overSamples; ++overSample)
		{
			float& in = channelBuffer[overSample];
			const float clipped = in < offsetGain ? offsetGain : in;
			in = dry * in + wet * clipped;
		}
#endif
	}

	// --- Downsample ---
	oversampler->processSamplesDown(block);

	// Low cut filter
	if (parametersValues[Parameters::LowCut] > 20.0f || parametersValues[Parameters::Offset] > 0.0f)
	{
		for (int channel = 0; channel < channels; channel++)
		{
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& lowCutFilter = m_lowCutFilter[channel];
			lowCutFilter.set(parametersValues[Parameters::LowCut]);

			for (int sample = 0; sample < samples; sample++)
			{
				float& in = channelBuffer[sample];
				in = lowCutFilter.process(in);
			}
		}
	}
}

//==============================================================================
bool HalfwaveSaturatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HalfwaveSaturatorAudioProcessor::createEditor()
{
    return new HalfwaveSaturatorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void HalfwaveSaturatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void HalfwaveSaturatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout HalfwaveSaturatorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Drive].paramName, m_parametersDescritpion[Parameters::Drive].paramName, NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Offset].paramName, m_parametersDescritpion[Parameters::Offset].paramName, NormalisableRange<float>(0.0f, 3.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::LowCut].paramName, m_parametersDescritpion[Parameters::LowCut].paramName, NormalisableRange<float>(20.0f, 120.0f, 1.0f, 1.0f), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>(-18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HalfwaveSaturatorAudioProcessor();
}
