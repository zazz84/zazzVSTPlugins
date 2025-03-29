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

const std::string LimiterAudioProcessor::paramsNames[] = { "Type", "Gain", "Release", "Threshold", "Volume" };
const std::string LimiterAudioProcessor::labelNames[] =  { "Type", "Input", "Release", "Threshold", "Output" };
const std::string LimiterAudioProcessor::paramsUnitNames[] = {"", " dB", " ms", " dB", " dB" };

const float LimiterAudioProcessor::MAXIMUM_ATTACK_TIME_MS = 1.0f;

//==============================================================================
LimiterAudioProcessor::LimiterAudioProcessor()
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
	typeParameter      = apvts.getRawParameterValue(paramsNames[0]);
	gainParameter      = apvts.getRawParameterValue(paramsNames[1]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[2]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);
}

LimiterAudioProcessor::~LimiterAudioProcessor()
{
}

//==============================================================================
const juce::String LimiterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LimiterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LimiterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LimiterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LimiterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LimiterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LimiterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LimiterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LimiterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	const int attackSize = (int)(MAXIMUM_ATTACK_TIME_MS * 0.001 * sampleRate);		// Maximum attack time
	
	setLatencySamples(attackSize);

	// Limiter 1
	const int attackSize1 = 2 * attackSize / 3;
	const int attackSize2 = attackSize - attackSize1;

	m_limiter1[0].init(sr, attackSize1);
	m_limiter1[1].init(sr, attackSize1);

	m_limiter2[0].init(sr, attackSize2);
	m_limiter2[1].init(sr, attackSize2);

	m_limiter1[0].setAttackSize(attackSize1);
	m_limiter1[1].setAttackSize(attackSize1);

	m_limiter2[0].setAttackSize(attackSize2);
	m_limiter2[1].setAttackSize(attackSize2);

	m_clipper[0].init(sr);
	m_clipper[1].init(sr);

	// Limiter 2
	m_logLimiter[0].init(sr, attackSize);
	m_logLimiter[1].init(sr, attackSize);

	m_logLimiter[0].setAttackSize(attackSize);
	m_logLimiter[1].setAttackSize(attackSize);

	m_clipper2[0].init(sr);
	m_clipper2[1].init(sr);

	// Limiter 3
	m_advancedLimiter[0].init(sr, attackSize);
	m_advancedLimiter[1].init(sr, attackSize);

	m_advancedLimiter[0].setAttackSize(attackSize);
	m_advancedLimiter[1].setAttackSize(attackSize);
}

void LimiterAudioProcessor::releaseResources()
{
	m_limiter1[0].release();
	m_limiter1[1].release();
	m_limiter2[0].release();
	m_limiter2[1].release();

	m_logLimiter[0].release();
	m_logLimiter[1].release();

	m_advancedLimiter[0].release();
	m_advancedLimiter[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LimiterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LimiterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto type			= typeParameter->load();
	const auto inputGain	= juce::Decibels::decibelsToGain(gainParameter->load());
	const auto release		= releaseParameter->load();
	const auto threshold	= juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto outputGain	= juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	
	buffer.applyGain(inputGain);

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		if (type == 1)
		{
			auto& limiter1 = m_limiter1[channel];
			auto& limiter2 = m_limiter2[channel];
			auto& clipper = m_clipper[channel];

			limiter1.setReleaseTime(release);
			limiter1.setThreshold(threshold);

			limiter2.setReleaseTime(release);
			limiter2.setThreshold(threshold);

			const float gainMin = limiter1.getGainMin() * limiter2.getGainMin();
			if (gainMin < m_gainMin)
			{
				m_gainMin = gainMin;
			}

			for (int sample = 0; sample < samples; sample++)
			{
				float in = channelBuffer[sample];

				in = limiter1.process(in);
				in = limiter2.process(in);

				const float out = clipper.process(in, threshold);

				channelBuffer[sample] = out;
			}
		}
		else if (type == 2)
		{
			auto& logLimiter = m_logLimiter[channel];
			
			logLimiter.setReleaseTime(release);
			logLimiter.setThreshold(threshold);

			const float gainMin = logLimiter.getGainMin();
			if (gainMin < m_gainMin)
			{
				m_gainMin = gainMin;
			}

			auto& clipper = m_clipper2[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				float in = channelBuffer[sample];

				const float out = clipper.process(logLimiter.process(in), threshold);

				channelBuffer[sample] = out;
			}
		}
		else
		{
			auto& limiter = m_advancedLimiter[channel];
			
			limiter.setReleaseTime(release);
			limiter.setThreshold(threshold);

			const float gainMin = limiter.getGainMin();
			if (gainMin < m_gainMin)
			{
				m_gainMin = gainMin;
			}

			for (int sample = 0; sample < samples; sample++)
			{
				float in = channelBuffer[sample];

				const float out = limiter.process(in);

				channelBuffer[sample] = out;
			}
		}
	}

	buffer.applyGain(outputGain);
}

//==============================================================================
bool LimiterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LimiterAudioProcessor::createEditor()
{
    return new LimiterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void LimiterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void LimiterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout LimiterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   3.0f,  1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   1.0f, 100.0f,  1.0f, 0.7f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -30.0f,   0.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LimiterAudioProcessor();
}
