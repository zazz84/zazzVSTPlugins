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

const std::string OctaverAudioProcessor::paramsNames[] = {     "GateThreshold", "GateSpeed", "EnvSpeed", "FrequencyMin", "FrequencyMax", "GeneratorSpeed", "GeneratorPitch", "GeneratorShape", "GeneratorVolume", "DirectVolume" };
const std::string OctaverAudioProcessor::labelNames[] = {      "Threshold",     "Speed",     "Speed",    "Min",          "Max",          "Speed",          "Pitch",          "Shape",          "Volume",          "Direct" };
const std::string OctaverAudioProcessor::paramsUnitNames[] = { " dB",           " %",        " %",       " Hz",          " Hz",          " %",             " st",            " %",             " dB",             " dB" };

//==============================================================================
OctaverAudioProcessor::OctaverAudioProcessor()
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
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		m_parameters[i] = apvts.getRawParameterValue(paramsNames[i]);
	}
}

OctaverAudioProcessor::~OctaverAudioProcessor()
{
}

//==============================================================================
const juce::String OctaverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OctaverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OctaverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OctaverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OctaverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OctaverAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OctaverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OctaverAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OctaverAudioProcessor::getProgramName (int index)
{
    return {};
}

void OctaverAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OctaverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_zeroCrossingRate.init(sr, 20.0f, 18000.0f);
	m_noiseGate.init(sr);
	m_oscilator.init(sr);
	m_frequencySmoother.init(sr);
	m_frequencySmoother.set(5.0f);
	m_inputEnvelope.init(sr);
}

void OctaverAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OctaverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void OctaverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get all params
	std::array<float, Parameters::COUNT> parametersValues;
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		parametersValues[i] = m_parameters[i]->load();
	}

	const auto frequencyMin = parametersValues[Parameters::FrequencyMin];
	const auto frequencyMax = std::fmaxf(frequencyMin, parametersValues[Parameters::FrequencyMax]);
	const auto speed = 0.5f + 49.5f * std::powf(0.01f * parametersValues[Parameters::Speed] , 2.0f);
	const auto pitch = parametersValues[Parameters::Pitch];
	const auto shape = std::powf(0.01f * parametersValues[Parameters::Shape], 2.0f);
	const auto generatorGain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);
	const auto directGain = parametersValues[Parameters::DirectVolume] <= -59.5f ? 0.0f : juce::Decibels::decibelsToGain(parametersValues[Parameters::DirectVolume]);

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = static_cast<int>(getSampleRate());
	const auto frequencyFadeStart = frequencyMin + 0.05f * (frequencyMax - frequencyMin);
	const auto pitchFator = pitch >= 0.0f ? 1.0f + pitch / 12.0f : -6.0f / pitch;
	const auto gateAttack =  Math::remap(parametersValues[Parameters::NoiseGateSpeed], 0.0f, 100.0f, 10.0f, 0.1f);
	const auto gateHold =    Math::remap(parametersValues[Parameters::NoiseGateSpeed], 0.0f, 100.0f, 200.0f, 1.0f);
	const auto gateRelease = Math::remap(parametersValues[Parameters::NoiseGateSpeed], 0.0f, 100.0f, 50.0f, 1.0f);
	const auto envelopeAttack = Math::remap(parametersValues[Parameters::EvnelopeSpeed], 0.0f, 100.0f, 50.0f, 0.1f);
	const auto envelopeRelease = Math::remap(parametersValues[Parameters::EvnelopeSpeed], 0.0f, 100.0f, 400.0f, 1.0f);

	// Setup
	m_zeroCrossingRate.set(frequencyMin, frequencyMax);
	m_frequencySmoother.set(speed);
	m_noiseGate.set(gateAttack, gateRelease, gateHold, parametersValues[Parameters::NoiseGateThreshold]);
	m_inputEnvelope.set(envelopeAttack, envelopeRelease);

	if (channels == 1)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Gate
			float out = m_noiseGate.process(in);

			// Input Envelope
			const float inputEnvelope = m_inputEnvelope.process(out);

			// Get zero crossing frequency
			const int zeroCrossingRate = m_zeroCrossingRate.process(out);
			const int zeroCrossingFrequency = out > 0.001f ? sampleRate / (2 * (2 + zeroCrossingRate)) : static_cast<int>(frequencyMin);

			// Oscilator
			const float frequencySmooth = m_frequencySmoother.process(std::clamp(static_cast<float>(zeroCrossingFrequency), frequencyMin, frequencyMax));
			m_oscilator.set(pitchFator * frequencySmooth, shape);
			out = m_oscilator.process();

			// Gain
			const float gain = Math::remap(frequencySmooth, frequencyMin, frequencyFadeStart, 0.0f, 1.0f);

			//Out
			channelBuffer[sample] = in * directGain + gain * inputEnvelope * generatorGain * out;
		}
	}
	else
	{
		// Channel pointer
		auto* leftChannelBuffer = buffer.getWritePointer(0);
		auto* rightChannelBuffer = buffer.getWritePointer(1);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float inLeft = leftChannelBuffer[sample];
			const float inRight = rightChannelBuffer[sample];
			const float in = 0.5f * (inLeft + inRight);

			// Gate
			float out = m_noiseGate.process(in);

			// Input Envelope
			const float inputEnvelope = m_inputEnvelope.process(out);

			// Get zero crossing frequency
			const int zeroCrossingRate = m_zeroCrossingRate.process(out);
			const int zeroCrossingFrequency = out > 0.001f ? sampleRate / (2 * (2 + zeroCrossingRate)) : static_cast<int>(frequencyMin);

			// Oscilator
			const float frequencySmooth = m_frequencySmoother.process(std::clamp(static_cast<float>(zeroCrossingFrequency), frequencyMin, frequencyMax));
			m_oscilator.set(pitchFator * frequencySmooth, shape);
			out = m_oscilator.process();

			// Gain
			const float gain = Math::remap(frequencySmooth, frequencyMin, frequencyFadeStart, 0.0f, 1.0f);

			//Out
			out = gain * inputEnvelope * generatorGain * out;
			leftChannelBuffer[sample] = inLeft * directGain + out;
			rightChannelBuffer[sample] = inRight * directGain + out;
		}
	}
}

//==============================================================================
bool OctaverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OctaverAudioProcessor::createEditor()
{
    return new OctaverAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void OctaverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void OctaverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout OctaverAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -60.0f,      0.0f,  1.0f, 1.0f), -60.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,    100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,    100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  20.0f,  16000.0f,  1.0f, 0.4f),  40.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  20.0f,  16000.0f,  1.0f, 0.4f), 440.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,    100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -48.0f,     48.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f,    100.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( -18.0f,     18.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>( -60.0f,      0.0f,  1.0f, 1.0f), -60.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OctaverAudioProcessor();
}
