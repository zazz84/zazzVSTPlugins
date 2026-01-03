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

const std::string GradientAudioProcessor::paramsNames[] = { "Type", "Amount", "Volume" };
const std::string GradientAudioProcessor::labelNames[] = { "Type", "Amount", "Volume" };
const std::string GradientAudioProcessor::paramsUnitNames[] = { "", " %", " dB" };

//==============================================================================
GradientAudioProcessor::GradientAudioProcessor()
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

GradientAudioProcessor::~GradientAudioProcessor()
{
}

//==============================================================================
const juce::String GradientAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GradientAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GradientAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GradientAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GradientAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GradientAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GradientAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GradientAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GradientAudioProcessor::getProgramName (int index)
{
    return {};
}

void GradientAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GradientAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_movingAverage[0].init(4 * sr);
	m_movingAverage[1].init(4 * sr);
}

void GradientAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GradientAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GradientAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// Hard limit
		if (parametersValues[Parameters::Type] == 1)
		{
			const float maxGradientdB = Math::remap(parametersValues[Parameters::Amount], 0.0f, 100.0f, -24.0f, -60.0f);
			const float maxGradient = juce::Decibels::decibelsToGain(maxGradientdB);
			float outLast = m_outLast[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				const float delta = in - outLast;

				if (delta > maxGradient)
				{
					outLast = outLast + maxGradient;
				}
				else if (delta < -maxGradient)
				{
					outLast = outLast - maxGradient;
				}
				else
				{
					outLast = in;
				}

				//Out
				channelBuffer[sample] = outLast;
			}

			m_outLast[channel] = outLast;
		}
		/*else if (parametersValues[Parameters::Type] == 2)
		{
			constexpr float factor = 1000.0f;
			const float maxGradientdB = Math::remap(parametersValues[Parameters::Amount], 0.0f, 100.0f, -24.0f, -60.0f);
			const float maxGradient = juce::Decibels::decibelsToGain(maxGradientdB);
			float outLast = m_outLast[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				const float delta = in - outLast;

				if (delta > maxGradient)
				{
					const float over = delta - maxGradient;
					outLast = outLast + maxGradient + over * std::tanhf(1.0f / (factor * over));
				}
				else if (delta < -maxGradient)
				{
					const float over = maxGradient - delta;
					outLast = outLast - maxGradient - over * std::tanhf(1.0f / (factor * over));
				}
				else
				{
					outLast = in;
				}

				//Out
				channelBuffer[sample] = outLast;
			}

			m_outLast[channel] = outLast;
		}*/
		else if (parametersValues[Parameters::Type] == 2)
		{
			auto& movingAverage = m_movingAverage[channel];
			const int size = (int)Math::remap(parametersValues[Parameters::Amount], 0.0f, 100.0f, 2.0f, (float)(0.1 * getSampleRate()));
			movingAverage.set(size);
			
			float inLast = m_outLast[channel];
			m_outLast[channel] = channelBuffer[samples - 1];
			
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				const float delta = in - inLast;
				const float deltaAverage = movingAverage.process(delta);

				//Out
				channelBuffer[sample] = inLast + deltaAverage;

				inLast = in;
			}
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]));
}

//==============================================================================
bool GradientAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GradientAudioProcessor::createEditor()
{
    return new GradientAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void GradientAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void GradientAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout GradientAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Type], paramsNames[Parameters::Type], NormalisableRange<float>( 1.0f,  2.0f,  1.0f, 1.0f),  1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Amount], paramsNames[Parameters::Amount], NormalisableRange<float>( 0.0f,  100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume], paramsNames[Parameters::Volume], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GradientAudioProcessor();
}
