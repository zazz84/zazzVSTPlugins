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

const std::string RumbleAudioProcessor::paramsNames[] = { "Threshold", "Lenght", "Pitch", "Delay", "Amount", "Volume" };
const std::string RumbleAudioProcessor::labelNames[] = { "Threshold", "Lenght", "Pitch", "Delay", "Amount", "Volume" };
const std::string RumbleAudioProcessor::paramsUnitNames[] = { " dB", " ms", " st", " ms", " dB", " dB" };

//==============================================================================
RumbleAudioProcessor::RumbleAudioProcessor()
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
	m_thresholdParameter    = apvts.getRawParameterValue(paramsNames[0]);
	m_lenghtParameter		= apvts.getRawParameterValue(paramsNames[1]);
	m_pitchParameter		= apvts.getRawParameterValue(paramsNames[2]);
	m_delayParameter		= apvts.getRawParameterValue(paramsNames[3]);
	m_amountParameter	    = apvts.getRawParameterValue(paramsNames[4]);
	m_volumeParameter	    = apvts.getRawParameterValue(paramsNames[5]);

	m_soloButtonParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("S"));
}

RumbleAudioProcessor::~RumbleAudioProcessor()
{
}

//==============================================================================
const juce::String RumbleAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RumbleAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RumbleAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RumbleAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RumbleAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RumbleAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RumbleAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RumbleAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RumbleAudioProcessor::getProgramName (int index)
{
    return {};
}

void RumbleAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RumbleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int size = (int)((float)sampleRate * 0.001f * MAX_LENGHT_MS);
	
	m_buffer[0].init(size);
	m_buffer[1].init(size);

	m_samplesToRead[0] = -480000L;
	m_samplesToRead[1] = -480000L;
	m_delaySamples[0] = -480000L;
	m_delaySamples[1] = -480000L;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{

	}

}

void RumbleAudioProcessor::releaseResources()
{
	m_buffer[0].release();
	m_buffer[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RumbleAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RumbleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Misc constants
	const auto sampleRate = (float)getSampleRate();
	
	// Get params
	const auto threshold = juce::Decibels::decibelsToGain(m_thresholdParameter->load());
	const auto cooldown = (long)(0.001f * m_lenghtParameter->load() * sampleRate);
	const auto pitch = m_pitchParameter->load();
	const auto delayTimeMS = m_delayParameter->load();
	const auto ammount = juce::Decibels::decibelsToGain(m_amountParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Get buttons
	const auto solo = m_soloButtonParameter->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto step = pitch > 0.0f ? 1.0f + pitch / MAX_PITCH : 1.0f + 0.5f * pitch / MAX_PITCH;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& buffer = m_buffer[channel];
		auto& transientBuffer = m_transientBuffer[channel];
		auto& samplesToRead = m_samplesToRead[channel];
		auto& delaySamples = m_delaySamples[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
			buffer.write(in);

			const float bufferOut = buffer.read();
			float transientOut = 0.0f;

			// Handle transient
			delaySamples--;
			
			if (delaySamples < 0L)
			{
				samplesToRead--;
			}
			
			if (samplesToRead >= 0L)
			{
				transientOut = ammount * transientBuffer.m_buffer[samplesToRead];
			}
			else if (samplesToRead < -cooldown)
			{
				if (bufferOut > threshold)
				{
					float delay = 0.0f;
					samplesToRead = (long)((float)m_latencySamples / step) - 1L;

					for (long i = 0L; i < samplesToRead; i++)
					{
						transientBuffer.m_buffer[i] = buffer.readDelayTriLinearInterpolation(delay);
						delay += step;
					}

					delaySamples = (long)(delayTimeMS * 0.001f * sampleRate);

					// Add attack and release
					// NOTE: Data sored in the buffer in reverse
					transientBuffer.applyAttack(0, (int)samplesToRead / 3);
					transientBuffer.applyRelease((int)samplesToRead - (int)samplesToRead / 20, (int)samplesToRead - 1);
				}
			}
		
			//Out
			if (solo)
			{
				channelBuffer[sample] = transientOut;
			}
			else
			{
				channelBuffer[sample] = bufferOut + transientOut;
			}
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool RumbleAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RumbleAudioProcessor::createEditor()
{
    return new RumbleAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void RumbleAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void RumbleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout RumbleAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -36.0f,    0.0f,  0.1f, 1.0f), -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  50.0f, MAX_LENGHT_MS,  1.0f, 0.7f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -MAX_PITCH, 0.0f,  0.1f, 1.0f),  -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f,  200.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,   18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f,   18.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("S", "S", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RumbleAudioProcessor();
}
