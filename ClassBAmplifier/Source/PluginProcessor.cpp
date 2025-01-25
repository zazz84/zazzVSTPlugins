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

const std::string ClassBAmplifierAudioProcessor::paramsNames[] = { "Color", "LP Frequency", "Drive", "Mix", "Volume" };
const std::string ClassBAmplifierAudioProcessor::paramsUnitNames[] = { "", " Hz", " dB", " %", " dB" };

//==============================================================================
ClassBAmplifierAudioProcessor::ClassBAmplifierAudioProcessor()
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
	colorParameter			= apvts.getRawParameterValue(paramsNames[0]);
	lpFrequencyParameter	= apvts.getRawParameterValue(paramsNames[1]);
	driveParameter			= apvts.getRawParameterValue(paramsNames[2]);
	mixParameter			= apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter			= apvts.getRawParameterValue(paramsNames[4]);
}

ClassBAmplifierAudioProcessor::~ClassBAmplifierAudioProcessor()
{
}

//==============================================================================
const juce::String ClassBAmplifierAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClassBAmplifierAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ClassBAmplifierAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ClassBAmplifierAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ClassBAmplifierAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClassBAmplifierAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ClassBAmplifierAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClassBAmplifierAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ClassBAmplifierAudioProcessor::getProgramName (int index)
{
    return {};
}

void ClassBAmplifierAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ClassBAmplifierAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_classBAmplifier[0].init(sr);
	m_classBAmplifier[1].init(sr);

	m_preemphasisFilter[0].init(sr);
	m_preemphasisFilter[1].init(sr);

	m_deemphasisFilter[0].init(sr);
	m_deemphasisFilter[1].init(sr);

	m_lpFilter[0].init(sr);
	m_lpFilter[1].init(sr);
}

void ClassBAmplifierAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClassBAmplifierAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ClassBAmplifierAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto color = -12.0f + (24.0f * 0.01f) * colorParameter->load();
	const auto lpFrequency = lpFrequencyParameter->load();
	const auto drive = juce::Decibels::decibelsToGain(driveParameter->load());
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load() - 6.0f);	// -6dB volume compensation

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const float dry = gain * (1.0f - mix);
	const float wet = gain * mix;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& classBAmplifier = m_classBAmplifier[channel];
		auto& preEmphasisFilter = m_preemphasisFilter[channel];
		auto& deeEmphasisFilter = m_deemphasisFilter[channel];
		auto& lpFilter = m_lpFilter[channel];
		
		classBAmplifier.set(drive);
		preEmphasisFilter.setLowShelf(660.0f, 0.707f, -color);
		deeEmphasisFilter.setLowShelf(660.0f, 0.707f, color);
		lpFilter.setLowPass(lpFrequency, 0.707f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Process
			float out = preEmphasisFilter.processDF1(in);	
			out = classBAmplifier.process(out);
			out = deeEmphasisFilter.processDF1(out);
			out = lpFilter.processDF1(out);
		
			//Out
			channelBuffer[sample] = dry * in + wet * out;
		}
	}
}

//==============================================================================
bool ClassBAmplifierAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ClassBAmplifierAudioProcessor::createEditor()
{
    return new ClassBAmplifierAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ClassBAmplifierAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ClassBAmplifierAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ClassBAmplifierAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(    0.0f,   100.0f,  1.0f, 1.0f),    50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 1000.0f, 20000.0f,  1.0f, 1.0f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  -30.0f,    30.0f,  1.0f, 1.0f),     0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(    0.0f,   100.0f,  1.0f, 1.0f),   100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  -18.0f,    18.0f,  1.0f, 1.0f),     0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClassBAmplifierAudioProcessor();
}
