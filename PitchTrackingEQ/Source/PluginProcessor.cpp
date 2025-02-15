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

const std::string PitchTrackingEQAudioProcessor::paramsNames[] =     { "FrequencyMin", "FrequencyMax", "Speed", "Pitch", "Q", "Gain", "Volume" };
const std::string PitchTrackingEQAudioProcessor::labelNames[] =      { "Min",          "Max",          "Speed", "Pitch", "Q", "Gain", "Volume" };
const std::string PitchTrackingEQAudioProcessor::paramsUnitNames[] = { " Hz",          " Hz",          "",      " st",   "",  " dB",  " dB" };

//==============================================================================
PitchTrackingEQAudioProcessor::PitchTrackingEQAudioProcessor()
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
	frequencyMinParameter    = apvts.getRawParameterValue(paramsNames[0]);
	frequencyMaxParameter    = apvts.getRawParameterValue(paramsNames[1]);
	speedParameter		     = apvts.getRawParameterValue(paramsNames[2]);
	frequencyOffsetParameter = apvts.getRawParameterValue(paramsNames[3]);
	qParameter               = apvts.getRawParameterValue(paramsNames[4]);
	gainParameter            = apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter          = apvts.getRawParameterValue(paramsNames[6]);

	buttonAParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonA"));
	buttonBParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonB"));
	buttonCParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonC"));
}

PitchTrackingEQAudioProcessor::~PitchTrackingEQAudioProcessor()
{
}

//==============================================================================
const juce::String PitchTrackingEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchTrackingEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchTrackingEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchTrackingEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PitchTrackingEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchTrackingEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchTrackingEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchTrackingEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PitchTrackingEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void PitchTrackingEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PitchTrackingEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_pitchDetection.init(sr);
	m_smoother.init(sr);

	m_filter[0].init(sr);
	m_filter[1].init(sr);
}

void PitchTrackingEQAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchTrackingEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PitchTrackingEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto frequencyMin = frequencyMinParameter->load();
	const auto frequencyMax = frequencyMaxParameter->load();
	const auto speed = 0.01f * speedParameter->load();
	const auto semitones = frequencyOffsetParameter->load();
	const auto q = qParameter->load();
	const auto filterGain = gainParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Configure
	m_pitchDetection.set(frequencyMin, frequencyMax);
	m_smoother.set(1.0f + 9.0f * speed);

	// Buttons
	const auto LP = buttonAParameter->get();
	const auto HP = buttonBParameter->get();
	const auto P = buttonCParameter->get();

	// Mono
	if (channels == 1)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);
		auto& filter = m_filter[0];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Process pitch detection
			m_pitchDetection.process(in);

			// Smooth
			m_frequency = Math::clamp(m_smoother.process(m_pitchDetection.getFrequency()), 20.0f, 20000.0f);
			const float freq = Math::clamp(Math::shiftFrequency(m_frequency, semitones), 20.0f, 20000.0f);

			// Filter
			if (LP)
			{
				filter.setLowPass(freq, q);
			}
			else if (HP)
			{
				filter.setHighPass(freq, q);
			}
			else
			{
				filter.setPeak(freq, q, filterGain);
			}
			
			// Set output
			channelBuffer[sample] = filter.processDF1(in);
		}
	}
	// Stereo
	else
	{
		// Channel pointer
		auto* channelBufferL = buffer.getWritePointer(0);
		auto* channelBufferR = buffer.getWritePointer(1);

		auto& filterL = m_filter[0];
		auto& filterR = m_filter[1];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float inL = channelBufferL[sample];
			const float inR = channelBufferR[sample];
			const float in = 0.5f * (inL + inR);

			// Process pitch detection
			m_pitchDetection.process(in);

			// Smooth
			m_frequency = Math::clamp(m_smoother.process(m_pitchDetection.getFrequency()), 20.0f, 20000.0f);
			const float freq = Math::clamp(Math::shiftFrequency(m_frequency, semitones), 20.0f, 20000.0f);

			// Filter
			if (LP)
			{
				filterL.setLowPass(freq, q);
				filterR.setLowPass(freq, q);
			}
			else if (HP)
			{
				filterL.setHighPass(freq, q);
				filterR.setHighPass(freq, q);
			}
			else
			{
				filterL.setPeak(freq, q, filterGain);
				filterR.setPeak(freq, q, filterGain);
			}

			// Set output
			channelBufferL[sample] = filterL.processDF1(inL);
			channelBufferR[sample] = filterR.processDF1(inR);
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool PitchTrackingEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PitchTrackingEQAudioProcessor::createEditor()
{
    return new PitchTrackingEQAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void PitchTrackingEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PitchTrackingEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout PitchTrackingEQAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  20.0f, 20000.0f, 10.0f, 0.5f),    20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  20.0f, 20000.0f, 10.0f, 0.5f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   100.0f,  1.0f, 1.0f),    50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -48.0f,    48.0f, 0.01f, 1.0f),     0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.1f,    12.0f,  0.1f, 1.0f),     1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.1f,    18.0f,  0.1f, 1.0f),     0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -18.0f,    18.0f,  0.1f, 1.0f),     0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonA", "ButtonA", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonB", "ButtonB", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonC", "ButtonC", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchTrackingEQAudioProcessor();
}
