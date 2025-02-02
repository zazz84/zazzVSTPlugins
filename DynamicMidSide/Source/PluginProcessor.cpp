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

const std::string DynamicMidSideAudioProcessor::paramsNames[] = { "Speed", "Width", "MGain", "SGain", "MPan", "SPan", "Volume" };
const std::string DynamicMidSideAudioProcessor::labelNames[] =  { "Speed", "Width", "Mid", "Side", "Mid", "Side", "Volume" };
const std::string DynamicMidSideAudioProcessor::paramsUnitNames[] =  { "", " %", " dB", " dB", "", "", " dB" };

//==============================================================================
DynamicMidSideAudioProcessor::DynamicMidSideAudioProcessor()
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
	speedParameter  = apvts.getRawParameterValue(paramsNames[0]);
	widthParameter  = apvts.getRawParameterValue(paramsNames[1]);
	mGainParameter  = apvts.getRawParameterValue(paramsNames[2]);
	sGainParameter  = apvts.getRawParameterValue(paramsNames[3]);
	mPanParameter   = apvts.getRawParameterValue(paramsNames[4]);
	sPanParameter   = apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[6]);
}

DynamicMidSideAudioProcessor::~DynamicMidSideAudioProcessor()
{
}

//==============================================================================
const juce::String DynamicMidSideAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DynamicMidSideAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DynamicMidSideAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DynamicMidSideAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DynamicMidSideAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DynamicMidSideAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DynamicMidSideAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DynamicMidSideAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DynamicMidSideAudioProcessor::getProgramName (int index)
{
    return {};
}

void DynamicMidSideAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DynamicMidSideAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (static_cast<int>(sampleRate));

	m_dynamicMidSide[0].init(sr);
	m_dynamicMidSide[1].init(sr);
	m_dynamicMidSide[2].init(sr);
	m_dynamicMidSide[3].init(sr);

	m_bandSplit[0].init(sr);
	m_bandSplit[1].init(sr);

	const float frequency1 = 220.0f;
	const float frequency2 = 1300.0f;
	const float frequency3 = 4000.0f;

	m_bandSplit[0].set(frequency1, frequency2, frequency3);
	m_bandSplit[1].set(frequency1, frequency2, frequency3);

	m_correlation.init(sr);
	m_balance.init(sr);
}

void DynamicMidSideAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DynamicMidSideAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DynamicMidSideAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	if (getTotalNumOutputChannels() != 2)
	{
		return;
	}

	// Get params
	const auto speed = 2.0f + 0.98f * (100.0f - speedParameter->load());
	const auto width = widthParameter->load();
	const auto mGain = juce::Decibels::decibelsToGain(mGainParameter->load());
	const auto sGain = juce::Decibels::decibelsToGain(sGainParameter->load());
	const auto mPan = mPanParameter->load();
	const auto sPan = sPanParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto samples = buffer.getNumSamples();

	auto* LChannel = buffer.getWritePointer(0);
	auto* RChannel = buffer.getWritePointer(1);

	m_correlationMin = 1.0f;

	// Set envelope
	const auto attack = 2.0f * speed;
	const auto release = 6.0f * speed;

	m_dynamicMidSide[0].set(attack, release, mGain, sGain, mPan, sPan, width);
	m_dynamicMidSide[1].set(attack, release, mGain, sGain, mPan, sPan, width);
	m_dynamicMidSide[2].set(attack, release, mGain, sGain, mPan, sPan, width);

	for (int sample = 0; sample < samples; sample++)
	{		
		float left = LChannel[sample];
		float right = RChannel[sample];

		float band1Left = 0.0f;
		float band2Left = 0.0f;
		float band3Left = 0.0f;
		float band4Left = 0.0f;

		float band1Right = 0.0f;
		float band2Right = 0.0f;
		float band3Right = 0.0f;
		float band4Right = 0.0f;
		
		m_bandSplit[0].process(left, band1Left, band2Left, band3Left, band4Left);
		m_bandSplit[1].process(right, band1Right, band2Right, band3Right, band4Right);

		m_dynamicMidSide[0].process(band1Left, band1Right);
		m_dynamicMidSide[1].process(band2Left, band2Right);
		m_dynamicMidSide[2].process(band3Left, band3Right);
		m_dynamicMidSide[3].process(band4Left, band4Right);

		const float outLeft = band1Left + band2Left + band3Left + band4Left;
		const float outRight = band1Right + band2Right + band3Right + band4Right;

		LChannel[sample] = outLeft;
		RChannel[sample] = outRight;
	
		// Get correlation
		const float correlation = m_correlation.process(outLeft, outRight);
		if (correlation < m_correlationMin)
		{
			m_correlationMin = correlation;
		}
	}

	m_balanceAvg = m_balance.processBlock(buffer);

	buffer.applyGain(volume);
}

//==============================================================================
bool DynamicMidSideAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DynamicMidSideAudioProcessor::createEditor()
{
    return new DynamicMidSideAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void DynamicMidSideAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DynamicMidSideAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout DynamicMidSideAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -36.0f,  36.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -36.0f,  36.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  -1.0f,   1.0f, 0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(  -1.0f,   1.0f, 0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[5], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DynamicMidSideAudioProcessor();
}
