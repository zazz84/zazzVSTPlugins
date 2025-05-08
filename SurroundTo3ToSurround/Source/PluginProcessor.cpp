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

const std::string SurroundTo3ToSurroundAudioProcessor::paramsNames[] = { "Type", "Volume" };
const std::string SurroundTo3ToSurroundAudioProcessor::paramsUnitNames[] = { "", " dB" };

//==============================================================================
SurroundTo3ToSurroundAudioProcessor::SurroundTo3ToSurroundAudioProcessor()
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
	typeParameter	= apvts.getRawParameterValue(paramsNames[0]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[1]);
}

SurroundTo3ToSurroundAudioProcessor::~SurroundTo3ToSurroundAudioProcessor()
{
}

//==============================================================================
const juce::String SurroundTo3ToSurroundAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SurroundTo3ToSurroundAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SurroundTo3ToSurroundAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SurroundTo3ToSurroundAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SurroundTo3ToSurroundAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SurroundTo3ToSurroundAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SurroundTo3ToSurroundAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SurroundTo3ToSurroundAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SurroundTo3ToSurroundAudioProcessor::getProgramName (int index)
{
    return {};
}

void SurroundTo3ToSurroundAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SurroundTo3ToSurroundAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
}

void SurroundTo3ToSurroundAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SurroundTo3ToSurroundAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    /*if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;*/

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SurroundTo3ToSurroundAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto type = typeParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Channels layout: left, right, centre, LFE, leftSurround, rightSurround
	if (channels != 6)
	{
		return;
	}

	auto* leftChannelBuffer = buffer.getWritePointer(0);
	auto* rightChannelBuffer = buffer.getWritePointer(1);
	auto* centreChannelBuffer = buffer.getWritePointer(2);
	auto* LFEChannelBuffer = buffer.getWritePointer(3);
	auto* leftSurroundChannelBuffer = buffer.getWritePointer(4);
	auto* rightSurroundChannelBuffer = buffer.getWritePointer(5);

	if (type == 1)
	{
		for (int sample = 0; sample < samples; sample++)
		{
			// Read 5.1
			const float left = leftChannelBuffer[sample];
			const float right = rightChannelBuffer[sample];
			const float centre = centreChannelBuffer[sample];
			const float leftSurround = leftSurroundChannelBuffer[sample];
			const float rightSurround = rightSurroundChannelBuffer[sample];

			// Downmix to 3.0
			const float L = 0.5f * left + 0.25f * centre + 0.25f * leftSurround;
			const float R = 0.5f * right + 0.25f * centre + 0.25f * rightSurround;
			const float B = 0.5f * leftSurround + 0.5f * rightSurround;

			// Upmix to 5.1
			const float leftUpmix = L;
			const float rightUpmix = R;
			const float centreUpmix = 0.5f * L + 0.5f * R;
			const float leftSurroundUpmix = 0.5f * L + 0.5f * B;
			const float rightSurroundUpmix = 0.5f * R + 0.5f * B;

			//Out
			leftChannelBuffer[sample] = leftUpmix;
			rightChannelBuffer[sample] = rightUpmix;
			centreChannelBuffer[sample] = centreUpmix;
			leftSurroundChannelBuffer[sample] = leftSurroundUpmix;
			rightSurroundChannelBuffer[sample] = rightSurroundUpmix;
		}
	}
	else if (type == 2)
	{		
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

			// Upmix to 5.1
			const float leftUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[0]);
			const float rightUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[1]);
			const float centreUpmix			= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[2]);
			const float leftSurroundUpmix	= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[3]);
			const float rightSurroundUpmix	= Ambisonic::decodeToSpeaker2D(bFormat, Ambisonic::speakers50Deg[4]);

			//Out
			leftChannelBuffer[sample] = leftUpmix;
			rightChannelBuffer[sample] = rightUpmix;
			centreChannelBuffer[sample] = centreUpmix;
			leftSurroundChannelBuffer[sample] = leftSurroundUpmix;
			rightSurroundChannelBuffer[sample] = rightSurroundUpmix;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SurroundTo3ToSurroundAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SurroundTo3ToSurroundAudioProcessor::createEditor()
{
    return new SurroundTo3ToSurroundAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SurroundTo3ToSurroundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SurroundTo3ToSurroundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SurroundTo3ToSurroundAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,  2.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SurroundTo3ToSurroundAudioProcessor();
}
