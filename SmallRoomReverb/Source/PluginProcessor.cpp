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

const std::string SmallRoomReverbAudioProcessor::paramsNames[] =		{	"ER Predelay",	"ER Size", "ER Damping",	"ER Width",	"LR Predelay", "LR Size",	"LR Damping",	"LR Width", "ER Volume",	"LR Volume",	"Mix",	"Volume", "Difuser Type", "Tank Type" };
const std::string SmallRoomReverbAudioProcessor::paramsUnitNames[] =	{	" ms",			" ms",		"",				" %",			" ms",			"",			"",				" %",		" dB",			" dB",			" %",	" dB", "", "" };

//==============================================================================
SmallRoomReverbAudioProcessor::SmallRoomReverbAudioProcessor()
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

	difuserTypeParameter = apvts.getRawParameterValue(paramsNames[12]);
	tankTypeParameter = apvts.getRawParameterValue(paramsNames[13]);
}

SmallRoomReverbAudioProcessor::~SmallRoomReverbAudioProcessor()
{
}

//==============================================================================
const juce::String SmallRoomReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmallRoomReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmallRoomReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmallRoomReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmallRoomReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmallRoomReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmallRoomReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void SmallRoomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmallRoomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_difuser[0].init(sr);
	m_difuser[1].init(sr);

	m_tank[0].init(sr);
	m_tank[1].init(sr);
}

void SmallRoomReverbAudioProcessor::releaseResources()
{
	m_difuser[0].release();
	m_difuser[1].release();

	m_tank[0].release();
	m_tank[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmallRoomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SmallRoomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto earlyReflectionsPredelay = ERpredelayParameter->load();
	const auto earlyReflectionsSize = ERsizeParameter->load();
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

	const auto difuserType = static_cast<Difuser::Type>((int)difuserTypeParameter->load() - 1);
	const auto tankType =    static_cast<Tank::Type>   ((int)tankTypeParameter->load() - 1);

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Process
	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& difuser = m_difuser[channel];
		auto& tank = m_tank[channel];
		difuser.set(difuserType);
		tank.set(tankType, lateReflectionsSize, lateReflectionsDamping);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Process reverb
			float out = difuser.process(in);

			out = tank.process(out);

			//Out
			channelBuffer[sample] = in - mix * (in - out);
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SmallRoomReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SmallRoomReverbAudioProcessor::createEditor()
{
    return new SmallRoomReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SmallRoomReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SmallRoomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SmallRoomReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f,  10.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),  0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),  0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,  80.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),  0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),  0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8],   NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9],   NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>( 1.0f,  4.0f,  1.0f,  1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[13], paramsNames[13], NormalisableRange<float>( 1.0f,  4.0f,  1.0f,  1.0f),   1.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmallRoomReverbAudioProcessor();
}
