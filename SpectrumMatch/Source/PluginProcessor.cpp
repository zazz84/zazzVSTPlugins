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

const std::string SpectrumMatchAudioProcessor::paramsNames[] =		{ "Attack", "Release", "50 Hz", "120 Hz", "380 Hz", "1000 Hz", "3300 Hz", "10000 Hz", "Volume" };
const std::string SpectrumMatchAudioProcessor::labelNames[] =		{ "Attack", "Release", "50 Hz", "120 Hz", "380 Hz", "1000 Hz", "3300 Hz", "10000 Hz", "Volume" };
const std::string SpectrumMatchAudioProcessor::paramsUnitNames[] =	{ " ms", " ms", " dB", " dB", " dB", " dB", " dB", " dB", " dB" };

//==============================================================================
SpectrumMatchAudioProcessor::SpectrumMatchAudioProcessor()
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

SpectrumMatchAudioProcessor::~SpectrumMatchAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumMatchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumMatchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumMatchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumMatchAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumMatchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumMatchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumMatchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumMatchAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumMatchAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumMatchAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumMatchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	/*for (size_t i = 0; i < BANDS_COUNT; i++)
	{
		m_detectionFilter[i].init(sr);
		m_detectionFilter[i].setBandPassPeakGain(FILTER_FREQUENCY[i], DETECTION_FILTER_Q[i]);

		m_applyfilter[i].init(sr);
		
		m_RMS[i].init(sr);
		m_RMS[i].set(1.0f, 20.0f);

		m_smoother[i].init(sr);
	}*/

	m_spectrumMatch[0].init(sr);
	m_spectrumMatch[1].init(sr);
}

void SpectrumMatchAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumMatchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectrumMatchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	// Get all params
	std::array<float, Parameters::COUNT> parametersValues;
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		parametersValues[i] = m_parameters[i]->load();
	}

	// Params
	SpectrumMatch::Params params{	parametersValues[Parameters::Attack],
									parametersValues[Parameters::Release],
									parametersValues[Parameters::Gain1],
									parametersValues[Parameters::Gain2],
									parametersValues[Parameters::Gain3],
									parametersValues[Parameters::Gain4],
									parametersValues[Parameters::Gain5],
									parametersValues[Parameters::Gain6] };


	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& spectrumMatch = m_spectrumMatch[channel];

		spectrumMatch.set(params);

		for (int sample = 0; sample < samples; sample++)
		{
			channelBuffer[sample] = spectrumMatch.process(channelBuffer[sample]);
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]));
}

//==============================================================================
bool SpectrumMatchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumMatchAudioProcessor::createEditor()
{
    return new SpectrumMatchAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SpectrumMatchAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SpectrumMatchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumMatchAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f, 800.0f,  0.1f, 0.4f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 800.0f,  0.1f, 0.4f), 200.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumMatchAudioProcessor();
}
