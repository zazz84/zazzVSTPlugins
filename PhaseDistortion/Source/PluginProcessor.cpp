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

const ModernRotarySlider::ParameterDescription PhaseDistortionAudioProcessor::m_parametersDescritpion[] =
{
	{ "Drive",
	  " %",
	  "Drive" },
	
	{ "Tone",
	  " %",
	  "Tone" },
	
	{ "Mix",
	" %",
	"Mix" },

	{ "Volume",
      " dB",
      "Volume" }
};

//==============================================================================
PhaseDistortionAudioProcessor::PhaseDistortionAudioProcessor()
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

PhaseDistortionAudioProcessor::~PhaseDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String PhaseDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaseDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhaseDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhaseDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhaseDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaseDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhaseDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaseDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhaseDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhaseDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhaseDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_phaseDistortion[channel].init(sr);
	}
}

void PhaseDistortionAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaseDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PhaseDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	auto parametersChanged = loadParameters();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto wet = 0.01f * getParameterValue(Parameters::Mix);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& phaseDistortion = m_phaseDistortion[channel];
		
		if (parametersChanged)
		{
			const auto driveGain = juce::Decibels::decibelsToGain((0.01f * 36.0f) * getParameterValue(Parameters::Drive));
			const auto filterGain = Math::remap(getParameterValue(Parameters::Tone), -100.0f, 100.0f, -36.0f, 36.0f);
			phaseDistortion.set(driveGain, filterGain);
		}

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
		
			// Process
			const float out = phaseDistortion.process(in);

			//Out
			channelBuffer[sample] = in + wet * (out - in);
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(getParameterValue(Parameters::Volume)));
}

//==============================================================================
bool PhaseDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhaseDistortionAudioProcessor::createEditor()
{
    return new PhaseDistortionAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void PhaseDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PhaseDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout PhaseDistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Drive].paramName, m_parametersDescritpion[Parameters::Drive].paramName, NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Tone].paramName, m_parametersDescritpion[Parameters::Tone].paramName, NormalisableRange<float>( -100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Mix].paramName, m_parametersDescritpion[Parameters::Mix].paramName, NormalisableRange<float>( 0.0f, 100.0f, 0.1f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseDistortionAudioProcessor();
}
