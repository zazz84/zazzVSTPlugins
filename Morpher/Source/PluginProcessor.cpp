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
const std::string MorpherAudioProcessor::paramsNames[] =		{ "InVolume", "SCVolume", "Morph", "Type", "Volume" };
const std::string MorpherAudioProcessor::labelNames[] =			{ "InVolume", "SCVolume", "Morph", "Type", "Volume" };
const std::string MorpherAudioProcessor::paramsUnitNames[] =	{ " dB", " dB", " %", "", " dB" };

//==============================================================================
MorpherAudioProcessor::MorpherAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
					   .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                       )
#endif
{
	for (int i = 0; i < Parameters::COUNT; i++)
    {
        m_parameters[i] = apvts.getRawParameterValue(paramsNames[i]);
    }
}

MorpherAudioProcessor::~MorpherAudioProcessor()
{
}

//==============================================================================
const juce::String MorpherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MorpherAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MorpherAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MorpherAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MorpherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MorpherAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MorpherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MorpherAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MorpherAudioProcessor::getProgramName (int index)
{
    return {};
}

void MorpherAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MorpherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_inFilter[channel].init(sr);
		m_scFilter[channel].init(sr);
		m_spectrumMorph[channel].init(sr);
	}
}

void MorpherAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MorpherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MorpherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all params
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }

    // Get params
	const auto gain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);
	const auto inGain = juce::Decibels::decibelsToGain(parametersValues[Parameters::InVolume]);
	const auto scGain = juce::Decibels::decibelsToGain(parametersValues[Parameters::SCVolume]);

	// Input buffer
	auto mainBus = getBus(true, 0);
	auto mainBuffer = mainBus->getBusBuffer(buffer);

	// Sidechain buffer
	auto sideChainBus = getBus(true, 1);

	if (sideChainBus == nullptr || !sideChainBus->isEnabled())
	{
		return;
	}

	auto sideChainBuffer = sideChainBus->getBusBuffer(buffer);

	// Mics constants
	const int outputChannels = getTotalNumOutputChannels();
	const int SCChannels = sideChainBuffer.getNumChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < outputChannels; channel++)
	{
		auto* mainChannelBuffer = mainBuffer.getWritePointer(channel);
		auto* sideChainChannelBuffer = sideChainBuffer.getWritePointer(std::min(channel, SCChannels));
		
		// Volume
		if (parametersValues[Parameters::Type] == 1)
		{
			const auto wet = Math::remap(parametersValues[Parameters::Morph], -100.0f, 100.0f, 0.0f, 1.0f);
			const auto dry = 1.0f - wet;

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = mainChannelBuffer[sample];
				const float sc = sideChainChannelBuffer[sample];

				//Out
				mainChannelBuffer[sample] = dry * inGain * in + wet * scGain * sc;
			}
		}
		// LP
		else if (parametersValues[Parameters::Type] == 2)
		{
			auto& inFilter = m_inFilter[channel];
			auto& scFilter = m_scFilter[channel];

			// TODO: Optimize
			const float melMin = Math::frequenyToMel(40.0f);
			const float melMax = Math::frequenyToMel(18000.0f);

			const float mel = Math::remap(parametersValues[Parameters::Morph], -100.0f, 100.0f, melMin, melMax);
			const float frequency = Math::melToFrequency(mel);

			inFilter.set(frequency);
			scFilter.set(frequency);

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = mainChannelBuffer[sample];
				const float sc = sideChainChannelBuffer[sample];

				//Out
				mainChannelBuffer[sample] = inGain * inFilter.processHP(in) + scGain * scFilter.processLP(sc);
			}
		}
		// HP
		else if (parametersValues[Parameters::Type] == 3)
		{
			auto& inFilter = m_inFilter[channel];
			auto& scFilter = m_scFilter[channel];

			// TODO: Optimize
			const float melMin = Math::frequenyToMel(40.0f);
			const float melMax = Math::frequenyToMel(18000.0f);

			const float mel = Math::remap(parametersValues[Parameters::Morph], -100.0f, 100.0f, melMax, melMin);
			const float frequency = Math::melToFrequency(mel);

			inFilter.set(frequency);
			scFilter.set(frequency);

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = mainChannelBuffer[sample];
				const float sc = sideChainChannelBuffer[sample];

				//Out
				mainChannelBuffer[sample] = inGain * inFilter.processLP(in) + scGain * scFilter.processHP(sc);
			}
		}
		// Spectrum
		else if (parametersValues[Parameters::Type] == 4)
		{
			auto& spectrumMorph = m_spectrumMorph[channel];
			const auto wet = Math::remap(parametersValues[Parameters::Morph], -100.0f, 100.0f, 0.0f, 1.0f);
			const auto dry = 1.0f - wet;
			SpectrumMorph::Params params{ 5.0f, 50.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0, true, true, true, true, true, true, wet };
			spectrumMorph.set(params);

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = mainChannelBuffer[sample];
				const float sc = sideChainChannelBuffer[sample];

				//Out
				float outIn{ 0.0f };
				float outSC{ 0.0f };
				spectrumMorph.process(in, sc, outIn, outSC);
				mainChannelBuffer[sample] = inGain * dry *  outIn + scGain * wet * outSC;
				//mainChannelBuffer[sample] = inGain * outIn;
			}
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool MorpherAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MorpherAudioProcessor::createEditor()
{
    return new MorpherAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MorpherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MorpherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MorpherAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::InVolume], paramsNames[Parameters::InVolume], NormalisableRange<float>	(  -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::SCVolume], paramsNames[Parameters::SCVolume], NormalisableRange<float>	(  -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Morph], paramsNames[Parameters::Morph], NormalisableRange<float>			( -100.0f, 100.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Type], paramsNames[Parameters::Type], NormalisableRange<float>			(    1.0f,   4.0f,  1.0f, 1.0f),  1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume], paramsNames[Parameters::Volume], NormalisableRange<float>		(  -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MorpherAudioProcessor();
}
