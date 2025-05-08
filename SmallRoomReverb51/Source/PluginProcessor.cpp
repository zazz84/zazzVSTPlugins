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

#include <array>
#include <functional>

#include "../../../zazzVSTPlugins/Shared/Utilities/Ambisonic.h"

//==============================================================================

const std::string SmallRoomReverb51AudioProcessor::paramsNames[] =		{	"ER Predelay",	"ER Size", "ER Damping", "ER Width",	"LR Predelay", "LR Size",	"LR Damping",	"LR Width", "ER Volume",	"LR Volume",	"Mix",	"Volume", "Type" };
const std::string SmallRoomReverb51AudioProcessor::paramsUnitNames[] =	{	" ms",			" ms",		"",				" %",			" ms",			"",			"",				" %",		" dB",			" dB",		" %",	" dB", "" };

//==============================================================================
SmallRoomReverb51AudioProcessor::SmallRoomReverb51AudioProcessor()
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
	ERpredelayParameter = apvts.getRawParameterValue(paramsNames[0]);
	ERsizeParameter = apvts.getRawParameterValue(paramsNames[1]);
	ERdampingParameter = apvts.getRawParameterValue(paramsNames[2]);
	ERwidthParameter = apvts.getRawParameterValue(paramsNames[3]);

	LRpredelayParameter = apvts.getRawParameterValue(paramsNames[4]);
	LRsizeParameter = apvts.getRawParameterValue(paramsNames[5]);
	LRdampingParameter = apvts.getRawParameterValue(paramsNames[6]);
	LRwidthParameter = apvts.getRawParameterValue(paramsNames[7]);

	ERvolumeParameter = apvts.getRawParameterValue(paramsNames[8]);
	LRvolumeParameter = apvts.getRawParameterValue(paramsNames[9]);
	mixParameter = apvts.getRawParameterValue(paramsNames[10]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[11]);

	typeParameter = apvts.getRawParameterValue(paramsNames[12]);
}

SmallRoomReverb51AudioProcessor::~SmallRoomReverb51AudioProcessor()
{
}

//==============================================================================
const juce::String SmallRoomReverb51AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmallRoomReverb51AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverb51AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverb51AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmallRoomReverb51AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmallRoomReverb51AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmallRoomReverb51AudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmallRoomReverb51AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmallRoomReverb51AudioProcessor::getProgramName (int index)
{
    return {};
}

void SmallRoomReverb51AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmallRoomReverb51AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		m_reverb[i].init(sr, i);
	}
}

void SmallRoomReverb51AudioProcessor::releaseResources()
{
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		m_reverb[i].release();
	}
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmallRoomReverb51AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::create5point1())
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

void SmallRoomReverb51AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto earlyReflectionsPredelay = ERpredelayParameter->load();
	const auto earlyReflectionsMS = ERsizeParameter->load();
	const auto earlyReflectionsDamping = ERdampingParameter->load();
	const auto earlyReflectionsWidth = 0.01f * ERwidthParameter->load();
	const auto earlyReflectionsGain = juce::Decibels::decibelsToGain(ERvolumeParameter->load());

	const auto lateReflectionsPredelay = LRpredelayParameter->load();
	const auto lateReflectionsSize = LRsizeParameter->load();
	const auto lateReflectionsDamping = LRdampingParameter->load();
	const auto lateReflectionsWidth = 0.01f * LRwidthParameter->load();
	const auto lateReflectionsGain = juce::Decibels::decibelsToGain(LRvolumeParameter->load());

	const auto mix = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	const auto type = typeParameter->load();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Channels layout: left, right, centre, LFE, leftSurround, rightSurround
	if (channels != 6)
	{
		return;
	}

	// Process
	// 5 channel reverb
	if (type == 1)
	{
		for (int channel = 0; channel < channels; channel++)
		{
			if (channel == 3)
			{
				continue;
			}

			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);

			auto& reverb = m_reverb[channel];
			reverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
				lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Process reverb
				const float out = reverb.process(in);

				//Out
				channelBuffer[sample] = in - mix * (in - out);
			}
		}
	}
	else if (type == 2)
	{
		auto* leftChannelBuffer = buffer.getWritePointer(0);
		auto* rightChannelBuffer = buffer.getWritePointer(1);
		auto* centreChannelBuffer = buffer.getWritePointer(2);
		auto* LFEChannelBuffer = buffer.getWritePointer(3);
		auto* leftSurroundChannelBuffer = buffer.getWritePointer(4);
		auto* rightSurroundChannelBuffer = buffer.getWritePointer(5);

		auto& leftReverb = m_reverb[0];
		auto& rightReverb = m_reverb[1];
		auto& backReverb = m_reverb[2];

		leftReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
		rightReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
		backReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read 5.1
			const float left = leftChannelBuffer[sample];
			const float right = rightChannelBuffer[sample];
			const float centre = centreChannelBuffer[sample];
			const float leftSurround = leftSurroundChannelBuffer[sample];
			const float rightSurround = rightSurroundChannelBuffer[sample];

			// Downmix to 3.0
			float L = 0.5f * left + 0.25f * centre + 0.25f * leftSurround;
			float R = 0.5f * right + 0.25f * centre + 0.25f * rightSurround;
			float B = 0.5f * leftSurround + 0.5f * rightSurround;

			// Process
			L = leftReverb.process(L);
			R = rightReverb.process(R);
			B = backReverb.process(B);

			// Upmix to 5.1
			const float leftUpmix = L;
			const float rightUpmix = R;
			const float centreUpmix = 0.5f * L + 0.5f * R;
			const float leftSurroundUpmix = 0.5f * L + 0.5f * B;
			const float rightSurroundUpmix = 0.5f * R + 0.5f * B;

			//Out		
			leftChannelBuffer[sample] = left - mix * (left - leftUpmix);
			rightChannelBuffer[sample] = right - mix * (right - rightUpmix);
			centreChannelBuffer[sample] = centre - mix * (centre - centreUpmix);
			leftSurroundChannelBuffer[sample] = leftSurround - mix * (leftSurround - leftSurroundUpmix);
			rightSurroundChannelBuffer[sample] = rightSurround - mix * (rightSurround - rightSurroundUpmix);
		}
	}
	else if (type == 3)
	{		
		auto* leftChannelBuffer = buffer.getWritePointer(0);
		auto* rightChannelBuffer = buffer.getWritePointer(1);
		auto* centreChannelBuffer = buffer.getWritePointer(2);
		auto* LFEChannelBuffer = buffer.getWritePointer(3);
		auto* leftSurroundChannelBuffer = buffer.getWritePointer(4);
		auto* rightSurroundChannelBuffer = buffer.getWritePointer(5);

		auto& leftReverb = m_reverb[0];
		auto& rightReverb = m_reverb[1];
		auto& backReverb = m_reverb[2];

		leftReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
		rightReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
		backReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
			lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read 5.1
			const float left = leftChannelBuffer[sample];
			const float right = rightChannelBuffer[sample];
			const float centre = centreChannelBuffer[sample];
			const float leftSurround = leftSurroundChannelBuffer[sample];
			const float rightSurround = rightSurroundChannelBuffer[sample];

			// Downmix to FOA
			float gain = 1.0f / sqrtf(5.0f);
			Ambisonic::BFormat bFormat;

			Ambisonic::encodeToAmbisonics2D(left,			Ambisonic::speakers50Deg[0], bFormat, gain);
			Ambisonic::encodeToAmbisonics2D(right,			Ambisonic::speakers50Deg[1], bFormat, gain);
			Ambisonic::encodeToAmbisonics2D(centre,			Ambisonic::speakers50Deg[2], bFormat, gain);
			Ambisonic::encodeToAmbisonics2D(leftSurround,	Ambisonic::speakers50Deg[3], bFormat, gain);
			Ambisonic::encodeToAmbisonics2D(rightSurround,	Ambisonic::speakers50Deg[4], bFormat, gain);

			// Process
			bFormat.W = leftReverb.process(bFormat.W);
			bFormat.X = rightReverb.process(bFormat.X);
			bFormat.Y = backReverb.process(bFormat.Y);

			// Upmix to 5.1
			const float leftUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[0]);
			const float rightUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[1]);
			const float centreUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[2]);
			const float leftSurroundUpmix	= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[3]);
			const float rightSurroundUpmix	= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[4]);

			//Out		
			leftChannelBuffer[sample] = left - mix * (left - leftUpmix);
			rightChannelBuffer[sample] = right - mix * (right - rightUpmix);
			centreChannelBuffer[sample] = centre - mix * (centre - centreUpmix);
			leftSurroundChannelBuffer[sample] = leftSurround - mix * (leftSurround - leftSurroundUpmix);
			rightSurroundChannelBuffer[sample] = rightSurround - mix * (rightSurround - rightSurroundUpmix);
		}

		buffer.applyGain(juce::Decibels::decibelsToGain(-8.0f));
	}
	else if (type == 4)
	{
	auto* leftChannelBuffer = buffer.getWritePointer(0);
	auto* rightChannelBuffer = buffer.getWritePointer(1);
	auto* centreChannelBuffer = buffer.getWritePointer(2);
	auto* LFEChannelBuffer = buffer.getWritePointer(3);
	auto* leftSurroundChannelBuffer = buffer.getWritePointer(4);
	auto* rightSurroundChannelBuffer = buffer.getWritePointer(5);

	auto& leftReverb = m_reverb[0];
	auto& rightReverb = m_reverb[1];
	auto& backReverb = m_reverb[2];

	leftReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
		lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
	rightReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
		lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);
	backReverb.set(earlyReflectionsPredelay, earlyReflectionsMS, earlyReflectionsDamping, earlyReflectionsWidth, earlyReflectionsGain,
		lateReflectionsPredelay, lateReflectionsSize, lateReflectionsDamping, lateReflectionsWidth, lateReflectionsGain);

	for (int sample = 0; sample < samples; sample++)
	{
		// Read 5.1
		const float left = leftChannelBuffer[sample];
		const float right = rightChannelBuffer[sample];
		const float centre = centreChannelBuffer[sample];
		const float leftSurround = leftSurroundChannelBuffer[sample];
		const float rightSurround = rightSurroundChannelBuffer[sample];

		// Downmix to FOA
		float gain = 1.0f / sqrtf(5.0f);
		Ambisonic::BFormat bFormat;

		Ambisonic::encodeToAmbisonics2D(left, Ambisonic::speakers50Deg[0], bFormat, gain);
		Ambisonic::encodeToAmbisonics2D(right, Ambisonic::speakers50Deg[1], bFormat, gain);
		Ambisonic::encodeToAmbisonics2D(centre, Ambisonic::speakers50Deg[2], bFormat, gain);
		Ambisonic::encodeToAmbisonics2D(leftSurround, Ambisonic::speakers50Deg[3], bFormat, gain);
		Ambisonic::encodeToAmbisonics2D(rightSurround, Ambisonic::speakers50Deg[4], bFormat, gain);

		// Process
		const float W = bFormat.W;
		bFormat.W = leftReverb.process(W);
		bFormat.X = rightReverb.process(lateReflectionsWidth * W + (1.0f - lateReflectionsWidth) * bFormat.X);
		bFormat.Y = backReverb.process(lateReflectionsWidth * W + (1.0f - lateReflectionsWidth) * bFormat.Y);

		// Upmix to 5.1
		const float leftUpmix = Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[0]);
		const float rightUpmix = Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[1]);
		const float centreUpmix = Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[2]);
		const float leftSurroundUpmix = Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[3]);
		const float rightSurroundUpmix = Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[4]);

		//Out		
		leftChannelBuffer[sample] = left - mix * (left - leftUpmix);
		rightChannelBuffer[sample] = right - mix * (right - rightUpmix);
		centreChannelBuffer[sample] = centre - mix * (centre - centreUpmix);
		leftSurroundChannelBuffer[sample] = leftSurround - mix * (leftSurround - leftSurroundUpmix);
		rightSurroundChannelBuffer[sample] = rightSurround - mix * (rightSurround - rightSurroundUpmix);
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(-8.0f));
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SmallRoomReverb51AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SmallRoomReverb51AudioProcessor::createEditor()
{
    return new SmallRoomReverb51AudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SmallRoomReverb51AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SmallRoomReverb51AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SmallRoomReverb51AudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f,  10.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   5.0f,  60.0f,  0.1f, 1.0f),   30.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,  80.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f, 100.0f,  1.0f,  1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8],   NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9],   NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>( -18.0f,  18.0f,  0.1f,  1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>( 1.0f, 4.0f,  1.0f,  1.0f), 1.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmallRoomReverb51AudioProcessor();
}
