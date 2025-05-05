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

//==============================================================================

const std::string ZeroCrossingGateAudioProcessor::paramsNames[] = { "Threshold", "Volume" };
const std::string ZeroCrossingGateAudioProcessor::paramsUnitNames[] = { "samples", " dB" };

//==============================================================================
ZeroCrossingGateAudioProcessor::ZeroCrossingGateAudioProcessor()
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
	thresholdParameter = apvts.getRawParameterValue(paramsNames[0]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[1]);
}

ZeroCrossingGateAudioProcessor::~ZeroCrossingGateAudioProcessor()
{
}

//==============================================================================
const juce::String ZeroCrossingGateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ZeroCrossingGateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ZeroCrossingGateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ZeroCrossingGateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ZeroCrossingGateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ZeroCrossingGateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ZeroCrossingGateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ZeroCrossingGateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ZeroCrossingGateAudioProcessor::getProgramName (int index)
{
    return {};
}

void ZeroCrossingGateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ZeroCrossingGateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_zeroCrossingRate[0].init(sr, 280.0f, 8000.0f);
	m_zeroCrossingRate[1].init(sr, 280.0f, 8000.0f);
	m_smoother[0].init(sr);
	m_smoother[1].init(sr);
	m_smoother[0].set(5.0f, 100.0f, 100.0f);
	m_smoother[1].set(5.0f, 100.0f, 100.0f);
}

void ZeroCrossingGateAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ZeroCrossingGateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ZeroCrossingGateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto threshold = thresholdParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& zeroCrossingRate = m_zeroCrossingRate[channel];
		auto& smoother = m_smoother[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Handle zero crossing
			const auto rate = zeroCrossingRate.process(in);
			const auto gain = rate > threshold ? 1.0f : 0.0f;
			const auto gainSmooth = smoother.process(gain);

			//Out
			channelBuffer[sample] = gainSmooth * in;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool ZeroCrossingGateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ZeroCrossingGateAudioProcessor::createEditor()
{
    return new ZeroCrossingGateAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ZeroCrossingGateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ZeroCrossingGateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ZeroCrossingGateAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,  100.0f,  0.1f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ZeroCrossingGateAudioProcessor();
}
