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

#include <immintrin.h>

//==============================================================================

const std::string SmallSpeakerSimulatorAudioProcessor::paramsNames[] = { "Type", "Tune", "Resonance", "Mix", "Volume" };
const std::string SmallSpeakerSimulatorAudioProcessor::paramsUnitNames[] = { "", "", " %", " %", "dB" };

//==============================================================================
SmallSpeakerSimulatorAudioProcessor::SmallSpeakerSimulatorAudioProcessor()
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
	typeParameter = apvts.getRawParameterValue(paramsNames[0]);
	tuneParameter  = apvts.getRawParameterValue(paramsNames[1]);
	resonanceParameter  = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter  = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter  = apvts.getRawParameterValue(paramsNames[4]);
}

SmallSpeakerSimulatorAudioProcessor::~SmallSpeakerSimulatorAudioProcessor()
{
}

//==============================================================================
const juce::String SmallSpeakerSimulatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmallSpeakerSimulatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmallSpeakerSimulatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmallSpeakerSimulatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmallSpeakerSimulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmallSpeakerSimulatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmallSpeakerSimulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmallSpeakerSimulatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmallSpeakerSimulatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SmallSpeakerSimulatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmallSpeakerSimulatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_spekaerSimulation[0].init(sr);
	m_spekaerSimulation[1].init(sr);

	// Resonance
	const int predelaySize = (int)(1.0 * 0.001 * sampleRate);
	const int reflectionsSize = (int)(11.0 * 0.001 * sampleRate);
	const int size = predelaySize + reflectionsSize;
	m_earlyReflections[0].init(size, sr, 0);
	m_earlyReflections[1].init(size, sr, 1);

	m_earlyReflections[0].set(0.3f, predelaySize, reflectionsSize, 0.0f, -10.0f);
	m_earlyReflections[1].set(0.3f, predelaySize, reflectionsSize, 0.0f, -10.0f);
}

void SmallSpeakerSimulatorAudioProcessor::releaseResources()
{
	m_spekaerSimulation[0].release();
	m_spekaerSimulation[1].release();

	m_earlyReflections[0].release();
	m_earlyReflections[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmallSpeakerSimulatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SmallSpeakerSimulatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto type = static_cast<int>(typeParameter->load());
	const auto tune = tuneParameter->load();
	const auto resonanceWet = 0.01f * resonanceParameter->load();
	const auto wet = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const float dry = 1.0f - wet;
	const float resonanceDry = 1.0f - resonanceWet;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//Filters references
		auto& speakerSimulation = m_spekaerSimulation[channel];
		auto& earlyReflections = m_earlyReflections[channel];
		speakerSimulation.set(type, tune);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
			
			// Early reflections
			float out = resonanceDry * in + resonanceWet * earlyReflections.process(in);

			// EQ
			out = speakerSimulation.process(out);

			//Out
			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SmallSpeakerSimulatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SmallSpeakerSimulatorAudioProcessor::createEditor()
{
    return new SmallSpeakerSimulatorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SmallSpeakerSimulatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SmallSpeakerSimulatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SmallSpeakerSimulatorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 1.0f, 9.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 50.0f, 200.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmallSpeakerSimulatorAudioProcessor();
}
