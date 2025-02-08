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
SpectrumAnalyzerAudioProcessor::SpectrumAnalyzerAudioProcessor()
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
	buttonAParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonA"));
	buttonBParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonB"));
	buttonCParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonC"));
}

SpectrumAnalyzerAudioProcessor::~SpectrumAnalyzerAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumAnalyzerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAnalyzerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAnalyzerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumAnalyzerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAnalyzerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumAnalyzerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAnalyzerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumAnalyzerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumAnalyzerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const auto sr = static_cast<int>(sampleRate);

	m_frequenycSpectrum[0].init(sr);
	m_frequenycSpectrum[1].init(sr);
}

void SpectrumAnalyzerAudioProcessor::releaseResources()
{
	m_frequenycSpectrum[0].release();
	m_frequenycSpectrum[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumAnalyzerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectrumAnalyzerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	// Buttons
	const auto buttonA = buttonAParameter->get();
	const auto buttonB = buttonBParameter->get();
	const auto buttonC = buttonCParameter->get();

	const auto channels = buffer.getNumChannels();
	const auto samples = buffer.getNumSamples();

	// Set type
	if (channels == 1)
	{
		m_type = 3;
	}
	else
	{
		if (buttonA)
		{
			m_type = 0;
		}
		else if (buttonB)
		{
			m_type = 1;
		}
		else
		{
			m_type = 2;
		}
	}
	
	// Process
	if (m_type == 2)
	{
		auto& frequencySpectrumM = m_frequenycSpectrum[0];
		auto& frequencySpectrumS = m_frequenycSpectrum[1];
		auto* channelDataL = buffer.getWritePointer(0);
		auto* channelDataR = buffer.getWritePointer(1);

		for (int sample = 0; sample < samples; sample++)
		{
			const float inL = channelDataL[sample];
			const float inR = channelDataR[sample];

			const float m = 0.5f * (inL + inR);
			const float s = 0.5f * (inL - inR);

			frequencySpectrumM.process(m);
			frequencySpectrumS.process(s);
		}
	}
	else
	{
		for (int channel = 0; channel < channels; channel++)
		{
			auto& frequencySpectrum = m_frequenycSpectrum[channel];
			auto* channelData = buffer.getWritePointer(channel);

			for (int sample = 0; sample < samples; sample++)
			{
				frequencySpectrum.process(channelData[sample]);
			}
		}
	}

}

//==============================================================================
bool SpectrumAnalyzerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumAnalyzerAudioProcessor::createEditor()
{
    return new SpectrumAnalyzerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SpectrumAnalyzerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SpectrumAnalyzerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumAnalyzerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonA", "ButtonA", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonB", "ButtonB", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonC", "ButtonC", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAnalyzerAudioProcessor();
}
