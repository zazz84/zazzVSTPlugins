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

#include <cstdint>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================

const std::string BitmaskCrusherAudioProcessor::paramsNames[] = { "Threshold", "Mask", "Mix", "Volume" };
const std::string BitmaskCrusherAudioProcessor::paramsUnitNames[] = { " dB", "", " %", " dB" };

//==============================================================================
BitmaskCrusherAudioProcessor::BitmaskCrusherAudioProcessor()
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
	thresholdParameter	= apvts.getRawParameterValue(paramsNames[0]);
	bitMaskParameter		= apvts.getRawParameterValue(paramsNames[1]);
	mixParameter		= apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[3]);
}

BitmaskCrusherAudioProcessor::~BitmaskCrusherAudioProcessor()
{
}

//==============================================================================
const juce::String BitmaskCrusherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BitmaskCrusherAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BitmaskCrusherAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BitmaskCrusherAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BitmaskCrusherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BitmaskCrusherAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BitmaskCrusherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BitmaskCrusherAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BitmaskCrusherAudioProcessor::getProgramName (int index)
{
    return {};
}

void BitmaskCrusherAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BitmaskCrusherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
}

void BitmaskCrusherAudioProcessor::releaseResources()
{
	m_bitcrusher[0].release();
	m_bitcrusher[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BitmaskCrusherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BitmaskCrusherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto threshold = Math::dBToGain(thresholdParameter->load());
	const auto bitMask = static_cast<std::int8_t>(bitMaskParameter->load());
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = Math::dBToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto wet = gain * mix;
	const auto dry = gain * (1.0f - mix);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// References
		auto& bitCrusher = m_bitcrusher[channel];
		bitCrusher.set(threshold, bitMask);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// BitCrusher
			const float out = bitCrusher.process(in);
		
			//Out
			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool BitmaskCrusherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BitmaskCrusherAudioProcessor::createEditor()
{
    return new BitmaskCrusherAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BitmaskCrusherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BitmaskCrusherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BitmaskCrusherAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -60.0f,   0.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   1.0f, 127.0f,  1.0f, 1.0f), 127.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BitmaskCrusherAudioProcessor();
}
