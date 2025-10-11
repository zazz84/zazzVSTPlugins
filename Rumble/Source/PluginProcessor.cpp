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

const std::string RumbleAudioProcessor::paramsNames[] = { "Threshold", "Lenght", "Pitch", "Filter", "Amount", "Volume" };
const std::string RumbleAudioProcessor::labelNames[] = { "Threshold", "Lenght", "Pitch", "Filter", "Amount", "Volume" };
const std::string RumbleAudioProcessor::paramsUnitNames[] = { " dB", " ms", " st", " Hz", " dB", " dB" };

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
	m_filterParameter		= apvts.getRawParameterValue(paramsNames[3]);
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
	
	m_attactSamples = (long)(0.005 * sampleRate);

	m_size = size;
	const int size2 = size + size;

	m_buffer = new float[size2];
	memset(m_buffer, 0, size2 * sizeof(float));
	
	m_writteIndex[0] = (long)(size2 - 1);
	m_writteIndex[1] = (long)(size2 - 1);

	m_filter[0].init(size);
	m_filter[1].init(size);
}

void RumbleAudioProcessor::releaseResources()
{
	if (m_buffer != nullptr)
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}
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
	const auto envelopeLength = (long)(0.001f * m_lenghtParameter->load() * sampleRate);
	const auto pitch = m_pitchParameter->load();
	const auto frequency = m_filterParameter->load();
	const auto ammount = juce::Decibels::decibelsToGain(m_amountParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Get buttons
	const auto noSolo = 1.0f - (float)m_soloButtonParameter->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto step = powf(2.0f, pitch / 12.0f);
	const auto fadeOutStart = envelopeLength / 2L;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		float* buffer = m_buffer + (channel * m_size);
		auto& writteIndex = m_writteIndex[channel];
		auto& readIndex = m_readIndex[channel];
		auto& filter = m_filter[channel];

		filter.setLowPass(frequency, 0.707f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Trigger
			if (in > threshold && writteIndex >= envelopeLength)
			{
				readIndex = 0L;
				writteIndex = 0L;
			}

			// Read buffer
			float out = 0.0f;
			
			if (writteIndex < envelopeLength)
			{
				buffer[writteIndex] = in;

				// Linear interpolation
				const int readIndexTrunc = (int)(readIndex);
				const float weight = readIndex - (float)readIndexTrunc;

				out = (1.0f - weight) * buffer[readIndexTrunc] + weight * buffer[readIndexTrunc + 1];							
				out = filter.processDF1(out);

				// Trilinear
				/*const int readIndexTrunc = (int)(readIndex);
				const float weight = readIndex - (float)readIndexTrunc;

				const float yz1 = buffer[readIndexTrunc - 1];
				const float y0 = buffer[readIndexTrunc];
				const float y1 = buffer[readIndexTrunc + 1];
				const float y2 = buffer[readIndexTrunc + 2];

				// 4-point, 2nd-order Watte tri-linear (x-form)
				float ym1py2 = yz1 + y2;
				float c0 = y0;
				float c1 = (3.0f / 2.0f) * y1 - (1.0f / 2.0f) * (y0 + ym1py2);
				float c2 = (1.0f / 2.0f) * (ym1py2 - y0 - y1);

				out = (c2 * weight + c1) * weight + c0;				
				out = filter.processDF1(out);*/

				float gain = 1.0f;

				// Apply fade in
				if (writteIndex < m_attactSamples)
				{
					gain = Math::remap(writteIndex, 0.0f, m_attactSamples, 0.0f, 1.0f);
				}

				// Apply fade out
				if (writteIndex > fadeOutStart)
				{
					gain = Math::remap((float)writteIndex, (float)fadeOutStart, (float)envelopeLength, 1.0f, 0.0f);
					gain *= gain;
				}

				out *= gain;

				readIndex += step;
				writteIndex++;
			}

			channelBuffer[sample] = noSolo * in + ammount * out;
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
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   20.0f, 10000.0f,  1.0f, 0.4f), 10000.0f));
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
