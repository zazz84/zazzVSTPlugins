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

const std::string SampleAndHoldAudioProcessor::paramsNames[] = { "Threshold", "Frequency", "Mix", "Volume" };
const std::string SampleAndHoldAudioProcessor::paramsUnitNames[] = { " dB", " Hz", " %", " dB" };

//==============================================================================
SampleAndHoldAudioProcessor::SampleAndHoldAudioProcessor()
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
	thresholdParameter = apvts.getRawParameterValue(paramsNames[0]);
	frequencyParameter = apvts.getRawParameterValue(paramsNames[1]);
	mixParameter = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);
}

SampleAndHoldAudioProcessor::~SampleAndHoldAudioProcessor()
{
}

//==============================================================================
const juce::String SampleAndHoldAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SampleAndHoldAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SampleAndHoldAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SampleAndHoldAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SampleAndHoldAudioProcessor::getTailLengthSeconds() const
{
	// One sample to process one-pole filters
	return 1.0 / getSampleRate();
}

int SampleAndHoldAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SampleAndHoldAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SampleAndHoldAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SampleAndHoldAudioProcessor::getProgramName (int index)
{
    return {};
}

void SampleAndHoldAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SampleAndHoldAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_frequencySmoother[0].init(sr);
	m_frequencySmoother[1].init(sr);
	
	m_frequencySmoother[0].set(4.0);
	m_frequencySmoother[1].set(4.0);
}

void SampleAndHoldAudioProcessor::releaseResources()
{
	m_frequencySmoother[0].release();
	m_frequencySmoother[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SampleAndHoldAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SampleAndHoldAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto thresholdGain = Math::dBToGain(thresholdParameter->load());
	const auto frequency = frequencyParameter->load();
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = Math::dBToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = static_cast<float>(getSampleRate());
	const auto wet = gain * mix;
	const auto dry = gain * (1.0f - mix);

	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& holdValue = m_holdValue[channel];
		auto& samplesToHold = m_samplesToHold[channel];
		auto& frequencySmoother = m_frequencySmoother[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Smoothed frequency
			const float frequencySmooth = frequencySmoother.process(frequency);
			
			// Get input
			const float in = channelBuffer[sample];

			float out = 0.0f;

			if (samplesToHold <= 0)
			{
				if (in > thresholdGain || in < -thresholdGain)
				{
					out = holdValue = in;
					samplesToHold = static_cast<int>(sampleRate / frequencySmooth);
				}
				else
				{
					out = in;
				}
			}
			else
			{
				out = holdValue;
			}

			samplesToHold--;

			// Set output
			channelBuffer[sample] = dry * in + wet * out;
		}
	}
}

//==============================================================================
bool SampleAndHoldAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SampleAndHoldAudioProcessor::createEditor()
{
    return new SampleAndHoldAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SampleAndHoldAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SampleAndHoldAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SampleAndHoldAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -60.0f,     0.0f, 1.0f, 1.0f),	   -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  10.0f, 20000.0f, 1.0f, 0.4f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   100.0f, 1.0f, 1.0f),    100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,    18.0f, 1.0f, 1.0f),      0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleAndHoldAudioProcessor();
}
