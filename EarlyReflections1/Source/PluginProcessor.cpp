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

const std::string EarlyRefections1AudioProcessor::paramsNames[] =
											{
												"Predelay",
												"Length",
												"Decay",
												"Damping",
												"Diffusion",
												"Width",

												"Mix",
												"Volume"
											};
const std::string EarlyRefections1AudioProcessor::labelNames[] =
											{
												"Predelay",
												"Length",
												"Decay",
												"Damping",
												"Diffusion",
												"Width",

												"Mix",
												"Volume"
											};
const std::string EarlyRefections1AudioProcessor::paramsUnitNames[] =
											{
												" ms",
												" ms",
												" dB",
												" %",
												" %",
												" %",

												" %",
												" dB"
											};

//==============================================================================
EarlyRefections1AudioProcessor::EarlyRefections1AudioProcessor()
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
	m_predelayParameter = apvts.getRawParameterValue(paramsNames[0]);
	m_lengthParameter = apvts.getRawParameterValue(paramsNames[1]);
	m_decayParameter = apvts.getRawParameterValue(paramsNames[2]);
	m_dampingParameter = apvts.getRawParameterValue(paramsNames[3]);
	m_diffusionParameter = apvts.getRawParameterValue(paramsNames[4]);
	m_widthParameter = apvts.getRawParameterValue(paramsNames[5]);

	m_mixParameter = apvts.getRawParameterValue(paramsNames[6]);
	m_volumeParameter = apvts.getRawParameterValue(paramsNames[7]);
}

EarlyRefections1AudioProcessor::~EarlyRefections1AudioProcessor()
{
}

//==============================================================================
const juce::String EarlyRefections1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EarlyRefections1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EarlyRefections1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EarlyRefections1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EarlyRefections1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EarlyRefections1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EarlyRefections1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void EarlyRefections1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EarlyRefections1AudioProcessor::getProgramName (int index)
{
    return {};
}

void EarlyRefections1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EarlyRefections1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_earlyReflections[0].init(sr, 0);
	m_earlyReflections[1].init(sr, 1);
}

void EarlyRefections1AudioProcessor::releaseResources()
{
	m_earlyReflections[0].release();
	m_earlyReflections[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EarlyRefections1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EarlyRefections1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Prevent denormals for this function scope
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	EarlyReflectionsParams earlyReflectionsParams = {};
	earlyReflectionsParams.predelay		= m_predelayParameter->load();
	earlyReflectionsParams.length		= m_lengthParameter->load();
	earlyReflectionsParams.decay		= -m_decayParameter->load();
	earlyReflectionsParams.diffusion	= 0.01f * m_diffusionParameter->load();
	earlyReflectionsParams.damping		= 0.01f * m_dampingParameter->load();
	earlyReflectionsParams.width		= 0.01f * m_widthParameter->load();

	const auto mix = 0.01f * m_mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto gainCompensation = Math::dBToGain(Math::remap(earlyReflectionsParams.decay, -30.0f, 0.0f, -4.5f, -12.0f) + Math::remap(earlyReflectionsParams.length, 10.0f, 100.0f, 2.0f, 0.0f));

	for (int channel = 0; channel < channels; channel++)
	{
		auto& earlyReflections = m_earlyReflections[channel];
		earlyReflections.set(earlyReflectionsParams);

		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// Early reflections
			const float out = gainCompensation * earlyReflections.process(in);
		
			//Out
			channelBuffer[sample] = (1.0f - mix) * in + mix * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool EarlyRefections1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EarlyRefections1AudioProcessor::createEditor()
{
    return new EarlyRefections1AudioProcessorEditor (*this, apvts);
}

//==============================================================================
void EarlyRefections1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void EarlyRefections1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout EarlyRefections1AudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(0.0f, PREDELAY_TIME_MS,  1.0f, 1.0f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(10.0f, EarlyReflections::DELAY_LINE_LENGHT_MAX_MS - PREDELAY_TIME_MS, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(0.0f, 30.0f, 0.1f, 1.0f), 15.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(-18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EarlyRefections1AudioProcessor();
}
