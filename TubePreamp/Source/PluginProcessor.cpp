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

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================

const std::string TubePreampAudioProcessor::paramsNames[] = { "Drive", "Volume" };
const std::string TubePreampAudioProcessor::labelNames[] = { "Drive", "Volume" };
const std::string TubePreampAudioProcessor::paramsUnitNames[] = { " dB", " dB" };

//==============================================================================
TubePreampAudioProcessor::TubePreampAudioProcessor()
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
	m_driveParameter     = apvts.getRawParameterValue(paramsNames[0]);
	m_volumeParameter    = apvts.getRawParameterValue(paramsNames[1]);
}

TubePreampAudioProcessor::~TubePreampAudioProcessor()
{
}

//==============================================================================
const juce::String TubePreampAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TubePreampAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TubePreampAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TubePreampAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TubePreampAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TubePreampAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TubePreampAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TubePreampAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TubePreampAudioProcessor::getProgramName (int index)
{
    return {};
}

void TubePreampAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TubePreampAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = int(sampleRate);
	const float frequency = Math::fminf(19000.0f, (0.5f * 0.9f) * (float)sampleRate);

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_preFilter[channel].init(sr);
		m_postFilter[channel].init(sr);

		m_preFilter[channel].setHighPass(25.0f, 0.9f);
		m_postFilter[channel].setLowPass(frequency, 0.9f);

		m_envelopeFollower[channel].init(sr);
		m_envelopeFollower[channel].set(100.0f, 800.0f);
	}

	if (m_bias != nullptr)
	{
		delete[] m_bias;
		m_bias = nullptr;
	}

	m_bias = new float[samplesPerBlock];

	// Initialize  oversampling
	oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
		getTotalNumOutputChannels(),					// number of channels
		OVERSAMPLING_FACTOR,							// oversampling factor. E.g.2^4 = 16x
		juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
		true);											// use interpolation for better quality

	oversampler->reset();
	oversampler->initProcessing(samplesPerBlock);

	setLatencySamples((int)oversampler->getLatencyInSamples());
}

void TubePreampAudioProcessor::releaseResources()
{
	if (m_bias != nullptr)
	{
		delete[] m_bias;
		m_bias = nullptr;
	}
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TubePreampAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TubePreampAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto drivedB = m_driveParameter->load();
	const auto drive = juce::Decibels::decibelsToGain(drivedB);
	const auto volume = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const float drivePerStagedB = drivedB / (float)N_STAGES;
	const float drivePerStage = juce::Decibels::decibelsToGain(drivePerStagedB);

	// Upsample
	juce::dsp::AudioBlock<float> block(buffer);
	auto oversampledBlock = oversampler->processSamplesUp(block);

	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& envelopeFollower = m_envelopeFollower[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Get input
			float& in = channelBuffer[sample];

			const float envelope = envelopeFollower.process(drive * in);

			//const float bias = Math::remap(envelope, 0.1f, 0.6f, 0.00f, 0.15f);
			constexpr float xMin = 0.1f;
			constexpr float xMax = 0.6f;
			constexpr float yMin = 0.0f;
			constexpr float yMax = 0.15f;
			constexpr float slope = yMax / (xMax - xMin);	// 0.15 / 0.5
			constexpr float offset = -0.03f;				// -0.1 * 0.3
			float bias = envelope * slope + offset;
			bias = Math::clamp(bias, yMin, yMax);

			m_bias[sample] = bias;
		}

		auto* oversampleBuffer = oversampledBlock.getChannelPointer(channel);
		
		for (int sample = 0; sample < oversampledBlock.getNumSamples(); sample++)
		{
			// Get input
			float& in = oversampleBuffer[sample];

			const float bias = m_bias[sample / OVERSAMPLING_MULTIPLIER];

			float out = in;
			out = TubeEmulation::process(drivePerStage * out + bias);
			out = TubeEmulation::process(drivePerStage * out - bias);
			out = TubeEmulation::process(drivePerStage * out + bias);
			out = TubeEmulation::process(drivePerStage * out - bias);
			out = TubeEmulation::process(drivePerStage * out + bias);
			out = TubeEmulation::process(drivePerStage * out - bias);
			out = TubeEmulation::process(drivePerStage * out + bias);
			out = TubeEmulation::process(drivePerStage * out - bias);
			
			// Store output
			in = out;
		}
	}

	// Downsample
	oversampler->processSamplesDown(block);

	// LP + HP filters
	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& preFilter = m_preFilter[channel];
		auto& postFilter = m_postFilter[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Get input
			float& in = channelBuffer[sample];

			// Store output
			in = volume * postFilter.processDF1(preFilter.processDF1(in));
		}
	}
}

//==============================================================================
bool TubePreampAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TubePreampAudioProcessor::createEditor()
{
    return new TubePreampAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void TubePreampAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TubePreampAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout TubePreampAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f, 18.0f, 0.1f,  1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f, 18.0f, 0.1f,  1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TubePreampAudioProcessor();
}
