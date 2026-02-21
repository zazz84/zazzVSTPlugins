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

const ModernRotarySlider::ParameterDescription PhaseModulatorAudioProcessor::m_parametersDescritpion[] =
{
	{ "Depth",
	  " %",
	  "Depth" },

	{ "Frequency",
	  " Hz",
	  "Frequency" },

	{ "Feedback",
	" %",
	"Feedback" },

	{ "Mix",
	  " %",
	  "Mix" },

	{ "Volume",
      " dB",
      "Volume" }
};

//==============================================================================
PhaseModulatorAudioProcessor::PhaseModulatorAudioProcessor()
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

PhaseModulatorAudioProcessor::~PhaseModulatorAudioProcessor()
{
}

//==============================================================================
const juce::String PhaseModulatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaseModulatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhaseModulatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhaseModulatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhaseModulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaseModulatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhaseModulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaseModulatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhaseModulatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhaseModulatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhaseModulatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_phaseModulator[channel].init(sr);
	}
}

void PhaseModulatorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaseModulatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PhaseModulatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	loadParameters();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto wet = 0.01f * getParameterValue(Parameters::Mix);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& phaseModulator = m_phaseModulator[channel];
		phaseModulator.set(getParameterValue(Parameters::Frequency), 0.01f * getParameterValue(Parameters::Depth), 0.0098f * getParameterValue(Parameters::Feedback));

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Phase modulator
			const float out = phaseModulator.process(in);
		
			//Out
			channelBuffer[sample] = in + wet * (out - in);
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(getParameterValue(Parameters::Volume)));
}

//==============================================================================
bool PhaseModulatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhaseModulatorAudioProcessor::createEditor()
{
    return new PhaseModulatorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void PhaseModulatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PhaseModulatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout PhaseModulatorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Depth].paramName, m_parametersDescritpion[Parameters::Depth].paramName, NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Frequency].paramName, m_parametersDescritpion[Parameters::Frequency].paramName, NormalisableRange<float>( 0.1f, 20000.0f, 0.01f, 0.2f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Feedback].paramName, m_parametersDescritpion[Parameters::Feedback].paramName, NormalisableRange<float>( -100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Mix].paramName, m_parametersDescritpion[Parameters::Mix].paramName, NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseModulatorAudioProcessor();
}
