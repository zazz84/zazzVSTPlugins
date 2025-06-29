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
namespace
{
	inline float frequencyToSamples(const float frequency, const float sampleRate)
	{
		return sampleRate / (2.0f * frequency);
	}
}

//==============================================================================

const std::string ManualFlangerAudioProcessor::paramsNames[] =	{
																	"Frequency",
																	"Feedback",
																	"HighPassFrequency",
																	"LowPassFrequency",
																	"Mix",
																	"Volume"
																};
const std::string ManualFlangerAudioProcessor::labelNames[] =	{
																	"Frequency",
																	"Feedback",
																	"High Pass",
																	"Low Pass",
																	"Mix",
																	"Volume"
																};
const std::string ManualFlangerAudioProcessor::paramsUnitNames[] =	{
																		" Hz",
																		" %",
																		" Hz",
																		" Hz",
																		" %",
																		" dB"
																	};

//==============================================================================
ManualFlangerAudioProcessor::ManualFlangerAudioProcessor()
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
	m_frequencyParameter			= apvts.getRawParameterValue(paramsNames[0]);
	m_feedbackParameter				= apvts.getRawParameterValue(paramsNames[1]);
	m_highPassFrequencyParameter	= apvts.getRawParameterValue(paramsNames[2]);
	m_lowPassFrequencyParameter		= apvts.getRawParameterValue(paramsNames[3]);
	m_mixParameter					= apvts.getRawParameterValue(paramsNames[4]);
	m_volumeParameter				= apvts.getRawParameterValue(paramsNames[5]);
}

ManualFlangerAudioProcessor::~ManualFlangerAudioProcessor()
{
}

//==============================================================================
const juce::String ManualFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ManualFlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ManualFlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ManualFlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ManualFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ManualFlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ManualFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ManualFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ManualFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ManualFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ManualFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	const int delay = static_cast<int>(frequencyToSamples(20.0f, static_cast<float>(sampleRate)));

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_delayLine[channel].init(delay);
		m_highPassFilter[channel].init(sr);
		m_lowPassFilter[channel].init(sr);
		m_frequencySmoother[channel].init(sr);
		m_frequencySmoother[channel].set(10.0f);
	}
}

void ManualFlangerAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ManualFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ManualFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto frequency = m_frequencyParameter->load();
	const auto feedback = Math::remap(m_feedbackParameter->load(), 0.0f, 100.0f, -0.25f, 1.0f);
	const auto highPassFrequency = m_highPassFrequencyParameter->load();
	const auto lowPassFrequency = m_lowPassFrequencyParameter->load();
	const auto wet = 0.01f * m_mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto dry = 1.0f - wet;
	const auto sampleRate = static_cast<float>(getSampleRate());
	const auto delaySamples = frequencyToSamples(frequency, sampleRate);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& highPassFilter = m_highPassFilter[channel];
		auto& lowPassFilter = m_lowPassFilter[channel];
		auto& delayLine = m_delayLine[channel];
		auto& frequencySmoother = m_frequencySmoother[channel];

		highPassFilter.setHighPass(highPassFrequency, 1.0f);
		lowPassFilter.setLowPass(lowPassFrequency, 0.4f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Handle delay line
			const float delaySamplesSmooth = frequencySmoother.process(delaySamples);
			const float delayLineOut = delayLine.readDelayTriLinearInterpolation(delaySamplesSmooth);
			delayLine.write(in + feedback * delayLineOut);
		
			//Out
			channelBuffer[sample] = dry * in + wet * highPassFilter.processDF1(lowPassFilter.processDF1(delayLineOut));
		}

		buffer.applyGain(gain);
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool ManualFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ManualFlangerAudioProcessor::createEditor()
{
    return new ManualFlangerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ManualFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ManualFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ManualFlangerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  20.0f, 20000.0f,  1.0f, 0.3f),   100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,   100.0f,  1.0f, 1.0f),     0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  20.0f, 20000.0f,  1.0f, 0.3f),    20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  20.0f, 20000.0f,  1.0f, 0.3f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,   100.0f,  1.0f, 1.0f),    50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f,    18.0f,  0.1f, 1.0f),     0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ManualFlangerAudioProcessor();
}
