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

const std::string VolumeAudioProcessor::paramsNames[] = { "Volume" };
const std::string VolumeAudioProcessor::labelNames[] = { "Volume" };
const std::string VolumeAudioProcessor::paramsUnitNames[] = { " dB" };
const std::string VolumeAudioProcessor::buttonNames[] = { "-12", "-6", "+6", "+12" };

//==============================================================================
VolumeAudioProcessor::VolumeAudioProcessor()
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
	m_volumeParameter    = apvts.getRawParameterValue(paramsNames[0]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter(buttonNames[0]));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter(buttonNames[1]));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter(buttonNames[2]));
	button4Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter(buttonNames[3]));
}

VolumeAudioProcessor::~VolumeAudioProcessor()
{
}

//==============================================================================
const juce::String VolumeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VolumeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VolumeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VolumeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VolumeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VolumeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VolumeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VolumeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VolumeAudioProcessor::getProgramName (int index)
{
    return {};
}

void VolumeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VolumeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
}

void VolumeAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VolumeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VolumeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	auto gaindB = m_volumeParameter->load();

	// Get buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();
	const auto button4 = button4Parameter->get();

	if (button1)
	{
		gaindB -= 12.0f;
	}

	if (button2)
	{
		gaindB -= 6.0f;
	}

	if (button3)
	{
		gaindB += 6.0f;
	}

	if (button4)
	{
		gaindB += 12.0f;
	}

	// Apply gain
	if (std::fabsf(m_gaindBLast - gaindB) > 0.01f)
	{
		const auto samples = buffer.getNumSamples();
		buffer.applyGainRamp(0, samples, juce::Decibels::decibelsToGain(m_gaindBLast), juce::Decibels::decibelsToGain(gaindB));
		
	}
	else
	{
		buffer.applyGain(juce::Decibels::decibelsToGain(gaindB));
	}

	m_gaindBLast = gaindB;
}

//==============================================================================
bool VolumeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VolumeAudioProcessor::createEditor()
{
    return new VolumeAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void VolumeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void VolumeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout VolumeAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>(buttonNames[0], buttonNames[0], false));
	layout.add(std::make_unique<juce::AudioParameterBool>(buttonNames[1], buttonNames[1], false));
	layout.add(std::make_unique<juce::AudioParameterBool>(buttonNames[2], buttonNames[2], false));
	layout.add(std::make_unique<juce::AudioParameterBool>(buttonNames[3], buttonNames[3], false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VolumeAudioProcessor();
}
