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

namespace
{
	__forceinline float getAttenuationFactor(const float attenuation)
	{
		const float A = 19944.0f;
		const float B = 0.0546f;
		return A * std::exp(-B * attenuation);
	};

	__forceinline float getAbsorbtionFrequency(const float distance, const float absorbtion)
	{
		const auto d = distance * Math::remap(absorbtion, 0.0f, 400.0f, 0.0f, 4.0f);
		return std::sqrtf(Math::remap(d, 0.0f, 1000.0f, 0.0f, 0.99f));
	}
}

//==============================================================================

const std::string SpeedOfSoundAudioProcessor::paramsNames[] =
												{ 
													"Pan", 
													"Attenuation", 
													"Absorbtion", 
													"Distance", 
													"Volume" 
												};
const std::string SpeedOfSoundAudioProcessor::labelNames[] =
												{
													"Pan",
													"Attenuation",
													"Absorbtion",
													"Distance",
													"Volume"
												};
const std::string SpeedOfSoundAudioProcessor::paramsUnitNames[] =
												{
													" %",
													" %",
													" %",
													" m",
													" dB"
												};

//==============================================================================
SpeedOfSoundAudioProcessor::SpeedOfSoundAudioProcessor()
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
	m_panParameter		= apvts.getRawParameterValue(paramsNames[0]);
	m_attenuation		= apvts.getRawParameterValue(paramsNames[1]);
	m_absorbtion		= apvts.getRawParameterValue(paramsNames[2]);
	m_distanceParameter = apvts.getRawParameterValue(paramsNames[3]);
	m_volumeParameter   = apvts.getRawParameterValue(paramsNames[4]);
}

SpeedOfSoundAudioProcessor::~SpeedOfSoundAudioProcessor()
{
}

//==============================================================================
const juce::String SpeedOfSoundAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpeedOfSoundAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpeedOfSoundAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpeedOfSoundAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpeedOfSoundAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpeedOfSoundAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpeedOfSoundAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpeedOfSoundAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpeedOfSoundAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpeedOfSoundAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpeedOfSoundAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int size = (int)((float)sampleRate * MAXIMUM_DISTANCE / SPEED_OF_SOUND);

	m_delayLine[0].init(size);
	m_delayLine[1].init(size);

	const int sr = (int)sampleRate;
	m_panSmoother.init(sr);
	m_distanceSmoother.init(sr);

	m_panSmoother.set(2.0f);
	m_distanceSmoother.set(6.0f);

	m_absorbtionFilter[0].init(sr);
	m_absorbtionFilter[1].init(sr);
}

void SpeedOfSoundAudioProcessor::releaseResources()
{
	m_delayLine[0].release();
	m_delayLine[1].release();

	m_panSmoother.release();
	m_distanceSmoother.release();

	m_absorbtionFilter[0].release();
	m_absorbtionFilter[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpeedOfSoundAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpeedOfSoundAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto pan = Math::remap(m_panParameter->load(), -100.0f, 100.0f, 0.0f, 2.0f);
	const auto attenuation = m_attenuation->load();
	const auto absorbtion = m_absorbtion->load();
	const auto distance = m_distanceParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();

	const auto delaySizeFactor = sampleRate / SPEED_OF_SOUND;

	if (channels == 1)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const auto in = channelBuffer[sample];

			// Delay
			const auto distanceSmooth = m_distanceSmoother.process(distance);
			const auto attenuationGain = Math::getAmplitudeAttenuation(distanceSmooth, getAttenuationFactor(attenuation), 1.0f);

			m_absorbtionFilter[0].setCoef(getAbsorbtionFrequency(distanceSmooth, absorbtion));

			m_delayLine[0].write(m_absorbtionFilter[0].process(attenuationGain * in));
			
			const auto out = m_delayLine[0].readDelayTriLinearInterpolation(distanceSmooth * delaySizeFactor);

			//Out
			channelBuffer[sample] = out;
		}
	}
	if (channels == 2)
	{
		// Channel pointer
		auto* channelBufferLeft = buffer.getWritePointer(0);
		auto* channelBufferRight = buffer.getWritePointer(1);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const auto inLeft = channelBufferLeft[sample];
			const auto inRight = channelBufferRight[sample];


			// Delay
			const auto distanceSmooth = m_distanceSmoother.process(distance);
			const auto attenuationGain = Math::getAmplitudeAttenuation(distanceSmooth, getAttenuationFactor(attenuation), 1.0f);
			const auto panSmooth = m_panSmoother.process(pan);

			m_absorbtionFilter[0].setCoef(getAbsorbtionFrequency(distanceSmooth, absorbtion));
			m_absorbtionFilter[1].setCoef(getAbsorbtionFrequency(distanceSmooth, absorbtion));

			m_delayLine[0].write(m_absorbtionFilter[0].process(attenuationGain * inLeft * (2.0f - panSmooth)));
			m_delayLine[1].write(m_absorbtionFilter[1].process(attenuationGain * inRight * panSmooth));

			const auto outLeft = m_delayLine[0].readDelayTriLinearInterpolation(distanceSmooth * delaySizeFactor);
			const auto outRight = m_delayLine[1].readDelayTriLinearInterpolation(distanceSmooth * delaySizeFactor);

			//Out
			channelBufferLeft[sample] = outLeft;
			channelBufferRight[sample] = outRight;
		}
	}

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
		
			//Out
			channelBuffer[sample] = in;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SpeedOfSoundAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpeedOfSoundAudioProcessor::createEditor()
{
    return new SpeedOfSoundAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SpeedOfSoundAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SpeedOfSoundAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SpeedOfSoundAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -100.0f,           100.0f, 1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(    0.0f,           200.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(    0.0f,           400.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(    0.0f, MAXIMUM_DISTANCE, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  -18.0f,            18.0f, 0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpeedOfSoundAudioProcessor();
}
