/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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
const ModernRotarySlider::ParameterDescription DeharshAudioProcessor::m_parametersDescritpion[] =
{
	{ "Damping",
	  " dB",
	  "Damping" },

	{ "Presence",
	  " dB",
	  "Presence" },

	{ "Saturation",
	  " %",
	  "Saturation" },

	{ "Mix",
	" %",
	"Mix" },

	{ "Volume",
	  " dB",
	  "Volume" }
};


//==============================================================================
DeharshAudioProcessor::DeharshAudioProcessor()
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

DeharshAudioProcessor::~DeharshAudioProcessor()
{
}

//==============================================================================
const juce::String DeharshAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DeharshAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DeharshAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DeharshAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DeharshAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DeharshAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DeharshAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DeharshAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DeharshAudioProcessor::getProgramName (int index)
{
    return {};
}

void DeharshAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DeharshAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_dampingFilter[channel].init(sr);
		m_presenceHighShelfFilter[channel].init(sr);
		m_presencePeakFilter[channel].init(sr);

		m_preEmphasisFilter[channel].init(sr);
		m_deEmphasisFilter[channel].init(sr);
	}
}

void DeharshAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DeharshAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DeharshAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	std::array<float, Parameters::COUNT> parametersValues;
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		parametersValues[i] = m_parameters[i]->load();
	}

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	
	// Samples loop parameters
	const auto damping = -1.0f * parametersValues[Parameters::Damping];
	const auto gain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);
	auto wet = 0.01f * parametersValues[Parameters::Mix];
	auto dry = 1.0f - wet;
	wet *= gain;
	dry *= gain;
	const auto dampingFilterQ = 0.07f * damping + 3.0f;
	auto wetSaturation = 0.01f * parametersValues[Parameters::Saturation];
	auto drySaturation = 1.0f - wetSaturation;
	wetSaturation *= wet;
	wetSaturation *= 2.0f;		// Gain adjustment for saturation knob
	drySaturation *= wet;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& dampingFilter = m_dampingFilter[channel];
		auto& presenceHighShelfFilter = m_presenceHighShelfFilter[channel];
		auto& presencePeakFilter = m_presencePeakFilter[channel];
		
		auto& preEmphasisFilter = m_preEmphasisFilter[channel];
		auto& deEmphasisFilter = m_deEmphasisFilter[channel];

		dampingFilter.setPeak(3136.0f, dampingFilterQ, damping);
		presenceHighShelfFilter.setHighShelf(4186.0f, 1.0f, parametersValues[Parameters::Presence]);
		presencePeakFilter.setPeak(6272.0f, 1.0f, 0.33f * parametersValues[Parameters::Presence]);

		preEmphasisFilter.setLowShelf(440.0f, 0.707f, -9.0f);
		deEmphasisFilter.setLowShelf(440.0f, 0.707f, 9.0f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
			float out = in;

			// Filter
			out = dampingFilter.processDF1(out);
			out = presenceHighShelfFilter.processDF1(out);
			out = presencePeakFilter.processDF1(out);

			// Saturation
			float outSat = preEmphasisFilter.processDF1(out);
			
			// Reciprocal waveshaper
			const float absOutSat = std::abs(outSat);
			outSat = outSat / (1.0f + 8.0f * absOutSat);
			
			outSat = deEmphasisFilter.processDF1(outSat);

			out = drySaturation * out + wetSaturation * outSat;
		
			//Out
			channelBuffer[sample] = dry * in + out;
		}
	}
}

//==============================================================================
bool DeharshAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DeharshAudioProcessor::createEditor()
{
    return new DeharshAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void DeharshAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DeharshAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout DeharshAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Damping].paramName, m_parametersDescritpion[Parameters::Damping].paramName, NormalisableRange<float>( 0.0f, 20.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Presence].paramName, m_parametersDescritpion[Parameters::Presence].paramName, NormalisableRange<float>( 0.0f, 6.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Saturation].paramName, m_parametersDescritpion[Parameters::Saturation].paramName, NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Mix].paramName, m_parametersDescritpion[Parameters::Mix].paramName, NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DeharshAudioProcessor();
}
