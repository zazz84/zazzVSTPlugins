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

const std::string ThreeBandEQAudioProcessor::paramsNames[] = { "Low", "FreqLM", "Mid", "FreqMH", "High", "Volume" };
const std::string ThreeBandEQAudioProcessor::labelNames[] = { "Gain", "Frequency", "Gain", "Frequency", "Gain", "Volume" };
const std::string ThreeBandEQAudioProcessor::paramsUnitNames[] = { " dB", " Hz", " dB", " Hz", " dB", " dB" };

//==============================================================================
ThreeBandEQAudioProcessor::ThreeBandEQAudioProcessor()
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
	gainLowParameter          = apvts.getRawParameterValue(paramsNames[0]);
	frequencyLowMidParameter  = apvts.getRawParameterValue(paramsNames[1]);
	gainMidParameter          = apvts.getRawParameterValue(paramsNames[2]);
	frequencyMidHighParameter = apvts.getRawParameterValue(paramsNames[3]);
	gainHighParameter         = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter           = apvts.getRawParameterValue(paramsNames[5]);
}

ThreeBandEQAudioProcessor::~ThreeBandEQAudioProcessor()
{
}

//==============================================================================
const juce::String ThreeBandEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ThreeBandEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ThreeBandEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ThreeBandEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ThreeBandEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ThreeBandEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ThreeBandEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ThreeBandEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ThreeBandEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ThreeBandEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ThreeBandEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	auto sr = static_cast<int>(sampleRate);
	m_lowMidFilter[0].init(sr);
	m_lowMidFilter[1].init(sr);
	m_midHighFilter[0].init(sr);
	m_midHighFilter[1].init(sr);
	m_allPassFilter[0].init(sr);
	m_allPassFilter[1].init(sr);
}

void ThreeBandEQAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ThreeBandEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ThreeBandEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto gainLow = juce::Decibels::decibelsToGain(gainLowParameter->load());
	const auto frequencyLowMid = frequencyLowMidParameter->load();
	const auto gainMid = juce::Decibels::decibelsToGain(gainMidParameter->load());
	const auto frequencyMidHigh = frequencyMidHighParameter->load();
	const auto gainHigh = juce::Decibels::decibelsToGain(gainHighParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& lowMifFilter = m_lowMidFilter[channel];
		auto& midHighFilter = m_midHighFilter[channel];
		auto& allPassFilter = m_allPassFilter[channel];

		lowMifFilter.set(frequencyLowMid);
		midHighFilter.set(frequencyMidHigh);
		allPassFilter.set(frequencyMidHigh);

		for (int sample = 0; sample < samples; ++sample)
		{
			// Get input
			const float in = channelBuffer[sample];

			const float low = lowMifFilter.processLP(in);
			const float midHigh = lowMifFilter.processHP(in);
			const float mid = midHighFilter.processLP(midHigh);
			const float high = midHighFilter.processHP(midHigh);
			const float lowAllPass = allPassFilter.process(low);

			// Apply volume, mix and send to output
			channelBuffer[sample] = gainLow * lowAllPass + gainMid * mid + gainHigh * high;
		}		
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool ThreeBandEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ThreeBandEQAudioProcessor::createEditor()
{
    return new ThreeBandEQAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ThreeBandEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ThreeBandEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ThreeBandEQAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  -18.0f,   18.0f, 0.1f, 1.0f),    0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(      80,     880, 1.0f, 0.4f),  440.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  -18.0f,   18.0f, 0.1f, 1.0f),    0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 1760.0f, 7040.0f, 1.0f, 0.4f), 3520.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  -18.0f,   18.0f, 0.1f, 1.0f),    0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(  -18.0f,   18.0f, 0.1f, 1.0f),    0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThreeBandEQAudioProcessor();
}
