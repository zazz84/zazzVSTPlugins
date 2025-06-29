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

const std::string InfrasonicFilterAudioProcessor::paramsNames[] =
														{	
															"Slope",
															"Frequency",
															"Volume"
														};
const std::string InfrasonicFilterAudioProcessor::labelNames[] =
														{
															"Slope",
															"Frequency",
															"Volume"
														};
const std::string InfrasonicFilterAudioProcessor::paramsUnitNames[] =
														{
															" dB/oct",
															" Hz",
															" dB"
														};

//==============================================================================
InfrasonicFilterAudioProcessor::InfrasonicFilterAudioProcessor()
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
	m_slopeParameter	 = apvts.getRawParameterValue(paramsNames[0]);
	m_frequencyParameter = apvts.getRawParameterValue(paramsNames[1]);
	m_volumeParameter    = apvts.getRawParameterValue(paramsNames[2]);
}

InfrasonicFilterAudioProcessor::~InfrasonicFilterAudioProcessor()
{
}

//==============================================================================
const juce::String InfrasonicFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool InfrasonicFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool InfrasonicFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool InfrasonicFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double InfrasonicFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int InfrasonicFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int InfrasonicFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void InfrasonicFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String InfrasonicFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void InfrasonicFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void InfrasonicFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_filter[0].init(sr);
	m_filter[1].init(sr);
}

void InfrasonicFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool InfrasonicFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void InfrasonicFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	
	// Get params
	const auto slope = m_slopeParameter->load();
	const auto frequency = fminf(m_frequencyParameter->load(), 0.45f * sampleRate);
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& filter = m_filter[channel];

		filter.set(frequency, (int)(std::roundf(slope / 6.0f)), MultiOrderHighPassFilter::FilterType::LowPass);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
		
			//Out
			channelBuffer[sample] = filter.process(in);
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool InfrasonicFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* InfrasonicFilterAudioProcessor::createEditor()
{
    return new InfrasonicFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void InfrasonicFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void InfrasonicFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout InfrasonicFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(    12.0f,     48.0f, 12.0f, 1.0f),    12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 10000.0f,  96000.0f,  1.0f, 1.0f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   -18.0f,     18.0f,  0.1f, 1.0f),     0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InfrasonicFilterAudioProcessor();
}
