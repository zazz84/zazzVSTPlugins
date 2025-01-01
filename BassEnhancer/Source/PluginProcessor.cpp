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

#include <math.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

//==============================================================================

const std::string BassEnhancerAudioProcessor::paramsNames[] = { "Frequency", "Amount", "Drive", "Volume" };
const std::string BassEnhancerAudioProcessor::paramsUnitNames[] = { "Hz", "%", " %", "dB" };

//==============================================================================
BassEnhancerAudioProcessor::BassEnhancerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
	frequencyParameter	= apvts.getRawParameterValue(paramsNames[0]);
	amountParameter		= apvts.getRawParameterValue(paramsNames[1]);
	driveParameter		= apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[3]);
}

BassEnhancerAudioProcessor::~BassEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String BassEnhancerAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool BassEnhancerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool BassEnhancerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool BassEnhancerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double BassEnhancerAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int BassEnhancerAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int BassEnhancerAudioProcessor::getCurrentProgram()
{
	return 0;
}

void BassEnhancerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String BassEnhancerAudioProcessor::getProgramName(int index)
{
	return {};
}

void BassEnhancerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void BassEnhancerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_peakFilter[0].init(sr);
	m_peakFilter[1].init(sr);

	m_lowShelfFilter[0].init(sr);
	m_lowShelfFilter[1].init(sr);

	m_lowShelfFilter2[0].init(sr);
	m_lowShelfFilter2[1].init(sr);

	m_splitFilter[0].init(sr);
	m_splitFilter[1].init(sr);
}

void BassEnhancerAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BassEnhancerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
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

void BassEnhancerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	// Get params
	const auto frequency = frequencyParameter->load();
	const auto amount = 0.01f * amountParameter->load();
	const auto drive = 0.01f * driveParameter->load();
	const auto gain = Math::dBToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto driveDry = 1.0f - drive;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//Filters references
		auto& lowShelfFilter = m_lowShelfFilter[channel];
		auto& lowShelfFilter2 = m_lowShelfFilter2[channel];
		auto& peakFilter = m_peakFilter[channel];
		auto& splitFilter = m_splitFilter[channel];

		//Set filters
		lowShelfFilter.setLowShelf(2.0f * frequency, 0.5f, 36.0f * amount);
		lowShelfFilter2.setLowShelf(0.35f * frequency, 0.5f, -36.0f * amount);
		peakFilter.setPeak(0.35f * frequency, 1.0f, -18.0f * amount);
		splitFilter.set(2.0f * frequency);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Saturate
			float lowSplit = splitFilter.processLP(in);
			const float highSplit = splitFilter.processHP(in);
			lowSplit = driveDry * lowSplit + 0.125f * drive * std::atanf(8.0f * lowSplit);
			const float saturated = lowSplit + highSplit;

			// Handle low end boost
			float low = saturated;
			low = lowShelfFilter.processDF1(low);
			low = lowShelfFilter2.processDF1(low);
			low = peakFilter.processDF1(low);

			// Out
			channelBuffer[sample] = saturated + low;
		}
	}

	buffer.applyGain(0.5f * gain);
}

//==============================================================================
bool BassEnhancerAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BassEnhancerAudioProcessor::createEditor()
{
	return new BassEnhancerAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void BassEnhancerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BassEnhancerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BassEnhancerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(40.0f, 200.0f, 1.0f, 1.0f), 80.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(-18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new BassEnhancerAudioProcessor();
}