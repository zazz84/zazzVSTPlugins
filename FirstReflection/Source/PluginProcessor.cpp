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
const ModernRotarySlider::ParameterDescription FirstReflectionAudioProcessor::m_parametersDescritpion[] =
{
	{ "ListenerY",
	  " m",
	  "ListenerY" },

	{ "EmitterY",
	  " m",
	  "EmitterY" },

	{ "EmitterX",
	  " m",
	  "EmitterX" },

	{ "Diffusion",
	" %",
	"Diffusion" },

	{ "Reflection",
	  " dB",
	  "Reflection" },

	{ "LP",
	  " Hz",
	  "LP" },

	{ "Volume",
	  " dB",
	  "Volume" }
};

const float FirstReflectionAudioProcessor::MAXIMUM_HEIGHT = 200.0f;
const float FirstReflectionAudioProcessor::MAXIMUM_DISTANCE = 2000.0f;
const float FirstReflectionAudioProcessor::MAXIMUM_DELAY_TIME = 0.060f;
const float FirstReflectionAudioProcessor::MAXIMUM_ALL_PASS_TIME = 0.0009f;

//==============================================================================
FirstReflectionAudioProcessor::FirstReflectionAudioProcessor()
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
        m_parameters[i] = apvts.getRawParameterValue(m_parametersDescritpion[i].paramName);
    }
}

FirstReflectionAudioProcessor::~FirstReflectionAudioProcessor()
{
}

//==============================================================================
const juce::String FirstReflectionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FirstReflectionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FirstReflectionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FirstReflectionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FirstReflectionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FirstReflectionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FirstReflectionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FirstReflectionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FirstReflectionAudioProcessor::getProgramName (int index)
{
    return {};
}

void FirstReflectionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FirstReflectionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	const float maximumDelaySample = MAXIMUM_DELAY_TIME * (float)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_delayLine[channel].init(maximumDelaySample);
		m_lowPassFilter[channel].init(sr);
		m_delaySamplesSmoother[channel].init(sr);
		m_delaySamplesSmoother[channel].set(2.0f);
		m_allPassFilter[channel].init((int)(MAXIMUM_ALL_PASS_TIME * (float)sampleRate));
		m_LFO[channel].init(sr);
		m_LFO[channel].set(1.2f);
	}
}

void FirstReflectionAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FirstReflectionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FirstReflectionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all params
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = getSampleRate();
	const float delayTimeSamples = std::fminf(MAXIMUM_DELAY_TIME, reflectionTimeDelay(parametersValues[Parameters::EmitterHeight], parametersValues[Parameters::ListenerHeight], parametersValues[Parameters::EmitterDistance])) * (float)getSampleRate();
	const float reflectionsGain = juce::Decibels::decibelsToGain(parametersValues[Parameters::ReflectionVolume]);
	const float difusedWet = 0.01f * parametersValues[Parameters::Diffusion];
	const float difusedDry = 1.0f - difusedWet;
	
	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& delayLine = m_delayLine[channel];
		auto& lowPassFilter = m_lowPassFilter[channel];
		auto& delaySamplesSmoother = m_delaySamplesSmoother[channel];
		auto& allPassFilter = m_allPassFilter[channel];
		auto& LFO = m_LFO[channel];
		
		lowPassFilter.set(parametersValues[Parameters::ReflectionLPCutoff]);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Get delayed sample
			const float delayTimeSamplesSmooth = delaySamplesSmoother.process(delayTimeSamples);
			const float delayedSample = delayLine.readDelay(delayTimeSamplesSmooth);
						
			// Modulated diffusion
			const float lfo = LFO.process();
			const int size0 = (0.4f + 0.6f * lfo) * MAXIMUM_ALL_PASS_TIME * sampleRate;
			allPassFilter.set(size0);
			const float out = (difusedDry * delayedSample) + (difusedWet * allPassFilter.process(delayedSample));

			delayLine.write(in);
		
			//Out
			channelBuffer[sample] = in + reflectionsGain * lowPassFilter.process(out);
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]));
}

//==============================================================================
bool FirstReflectionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FirstReflectionAudioProcessor::createEditor()
{
    return new FirstReflectionAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void FirstReflectionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void FirstReflectionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout FirstReflectionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::EmitterDistance].paramName, m_parametersDescritpion[Parameters::EmitterDistance].paramName, NormalisableRange<float>(-MAXIMUM_DISTANCE, MAXIMUM_DISTANCE,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::EmitterHeight].paramName, m_parametersDescritpion[Parameters::EmitterHeight].paramName, NormalisableRange<float>( 0.0f, MAXIMUM_HEIGHT,  0.1f, 1.0f),  100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::ListenerHeight].paramName, m_parametersDescritpion[Parameters::ListenerHeight].paramName, NormalisableRange<float>( 0.0f, 10.0f,  0.1f, 1.0f),  2.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Diffusion].paramName, m_parametersDescritpion[Parameters::Diffusion].paramName, NormalisableRange<float>( 0.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::ReflectionLPCutoff].paramName, m_parametersDescritpion[Parameters::ReflectionLPCutoff].paramName, NormalisableRange<float>( 100.0f,  20000.0f,  1.0f, 0.4f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::ReflectionVolume].paramName, m_parametersDescritpion[Parameters::ReflectionVolume].paramName, NormalisableRange<float>( -60.0f,  0.0f,  0.1f, 1.0f), -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FirstReflectionAudioProcessor();
}
