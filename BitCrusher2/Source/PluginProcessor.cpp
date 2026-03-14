/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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

//==============================================================================
const ModernRotarySlider::ParameterDescription BitCrusher2AudioProcessor::m_parametersDescritpion[] =
{
	{ "Depth",
	  " bit",
	  "Depth" },

	{ "Drive",
	" %",
	"Drive" },
	
	{ "Downsample",
	" x",
	"Downsample" },

	{ "Mix",
	" %",
	"Mix" },

	{ "Volume",
	" dB",
	"Volume" }
};

//==============================================================================
BitCrusher2AudioProcessor::BitCrusher2AudioProcessor()
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

BitCrusher2AudioProcessor::~BitCrusher2AudioProcessor()
{
}

//==============================================================================
const juce::String BitCrusher2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitCrusher2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BitCrusher2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BitCrusher2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BitCrusher2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitCrusher2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitCrusher2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitCrusher2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BitCrusher2AudioProcessor::getProgramName (int index)
{
    return {};
}

void BitCrusher2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BitCrusher2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	// Initialize  oversampling
	oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
		getTotalNumOutputChannels(),					// number of channels
		OVERSAMPLING_FACTOR,							// oversampling factor. E.g.2^4 = 16x
		juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
		true);											// use interpolation for better quality

	oversampler->reset();
	oversampler->initProcessing(samplesPerBlock);

	setLatencySamples((int)oversampler->getLatencyInSamples());

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_noiseGate[channel].init((int)(sampleRate * OVERSAMPLING_MULTIPLIER));
		m_noiseGate[channel].set(0.1f, 0.1f, 0.0f, -45.0f);
	}
}

void BitCrusher2AudioProcessor::releaseResources()
{
	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_bitCrusher[channel].release();
	}
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitCrusher2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BitCrusher2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	const bool paramsChanged = loadParameters();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = OVERSAMPLING_MULTIPLIER * buffer.getNumSamples();
	const auto gain = juce::Decibels::decibelsToGain(getParameterValue(Parameters::Volume));
	const auto wet = gain * 0.01f * getParameterValue(Parameters::Mix);
	const auto dry = gain * (1.0f - 0.01f * getParameterValue(Parameters::Mix));
	const auto downSample = OVERSAMPLING_MULTIPLIER * (int)(getSampleRate() / 48000.0) * (int)getParameterValue(Parameters::Downsample);

	// Upsample
	juce::dsp::AudioBlock<float> block(buffer);
	auto oversampledBlock = oversampler->processSamplesUp(block);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = oversampledBlock.getChannelPointer(channel);

		// References
		auto& bitCrusher = m_bitCrusher[channel];
		auto& noiseGate = m_noiseGate[channel];

		if (paramsChanged)
		{
			bitCrusher.set(getParameterValue(Parameters::BitDepth), downSample, 0.01f * getParameterValue(Parameters::Drive));
		}
	
		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// NoiseGate
			float out = noiseGate.process(in);
			
			// BitCrusher
			out = bitCrusher.process(out);

			//Out
			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	// Downsample
	oversampler->processSamplesDown(block);
}

//==============================================================================
bool BitCrusher2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitCrusher2AudioProcessor::createEditor()
{
    return new BitCrusher2AudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BitCrusher2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BitCrusher2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BitCrusher2AudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::BitDepth].paramName, m_parametersDescritpion[Parameters::BitDepth].paramName, NormalisableRange<float>( 0.0f, 8.0f, 0.1f, 1.0f), 4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Drive].paramName, m_parametersDescritpion[Parameters::Drive].paramName, NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Downsample].paramName, m_parametersDescritpion[Parameters::Downsample].paramName, NormalisableRange<float>( 1.0f, 65.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Mix].paramName, m_parametersDescritpion[Parameters::Mix].paramName, NormalisableRange<float>( 0.0f, 100.0f,  1.0f, 1.0f),   100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitCrusher2AudioProcessor();
}