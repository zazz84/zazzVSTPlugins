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

const std::string MidSideAudioProcessor::paramsNames[] = { "MGain", "SGain", "MPan", "SPan", "Volume" };
const std::string MidSideAudioProcessor::labelNames[] =  { "Mid", "Side", "Mid", "Side", "Volume" };
const std::string MidSideAudioProcessor::paramsUnitNames[] =  { " dB", " dB", "", "", " dB" };

//==============================================================================
MidSideAudioProcessor::MidSideAudioProcessor()
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
	mGainParameter = apvts.getRawParameterValue(paramsNames[0]);
	sGainParameter = apvts.getRawParameterValue(paramsNames[1]);
	mPanParameter = apvts.getRawParameterValue(paramsNames[2]);
	sPanParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

MidSideAudioProcessor::~MidSideAudioProcessor()
{
}

//==============================================================================
const juce::String MidSideAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidSideAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidSideAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidSideAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidSideAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidSideAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidSideAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidSideAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidSideAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidSideAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MidSideAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void MidSideAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidSideAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void MidSideAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	if (getTotalNumOutputChannels() != 2)
	{
		return;
	}

	// Get params
	const auto mGain = juce::Decibels::decibelsToGain(mGainParameter->load());
	const auto sGain = juce::Decibels::decibelsToGain(sGainParameter->load());
	const auto mPan = mPanParameter->load();
	const auto sPan = sPanParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto samples = buffer.getNumSamples();

	const float mPanL = volume * (0.5f - 0.5f * mPan);
	const float mPanR = volume * (0.5f + 0.5 * mPan);
	const float sPanL = volume * (0.5f - 0.5f * sPan);
	const float sPanR = volume * (0.5f + 0.5 * sPan);

	auto* LChannel = buffer.getWritePointer(0);
	auto* RChannel = buffer.getWritePointer(1);

	for (int sample = 0; sample < samples; sample++)
	{
		const float LIn = LChannel[sample];
		const float RIn = RChannel[sample];

		const float mid = mGain * (LIn + RIn);
		const float side = sGain * (LIn - RIn);
			
		LChannel[sample] = mPanL * mid + sPanL * side;
		RChannel[sample] = mPanR * mid - sPanR * side;
	}
}

//==============================================================================
bool MidSideAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidSideAudioProcessor::createEditor()
{
    return new MidSideAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MidSideAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MidSideAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MidSideAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidSideAudioProcessor();
}
