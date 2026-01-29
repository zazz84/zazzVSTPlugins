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

const std::string VocalFilterAudioProcessor::paramsNames[] =      { "Gain1",	"Gain2",	"Gain3",	"Gain4",	"Gain5",	"Volume" };
const std::string VocalFilterAudioProcessor::labelNames[] =		  { "80 Hz",	"250 Hz",	"660 Hz",	"3 kHz",	"8 kHz",	"Volume" };
const std::string VocalFilterAudioProcessor::paramsUnitNames[] =  { " dB",	" dB",		" dB",		" dB",		" dB",		" dB" };

//==============================================================================
VocalFilterAudioProcessor::VocalFilterAudioProcessor()
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

VocalFilterAudioProcessor::~VocalFilterAudioProcessor()
{
}

//==============================================================================
const juce::String VocalFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VocalFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VocalFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VocalFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VocalFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VocalFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VocalFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VocalFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VocalFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void VocalFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VocalFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	setLatencySamples(m_fftProcessor[0].getLatencyInSamples());

	m_fftProcessor[0].reset();
	m_fftProcessor[1].reset();

	m_fftProcessor[0].init(sr);
	m_fftProcessor[1].init(sr);
}

void VocalFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VocalFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VocalFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all params
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }

	//TEMP
	auto bypassed = false;

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& fftProcessor = m_fftProcessor[channel];
		fftProcessor.set(
			parametersValues[Parameters::Gain1],
			parametersValues[Parameters::Gain2],
			parametersValues[Parameters::Gain3],
			parametersValues[Parameters::Gain4],
			parametersValues[Parameters::Gain5]);

		fftProcessor.processBlock(channelBuffer, samples, bypassed);
		
		/*for (size_t sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
		
			// Process
			const float out = fftProcessor.processSample(in, bypassed);

			//Out
			channelBuffer[sample] = out;
		}*/
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]));
}

//==============================================================================
bool VocalFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VocalFilterAudioProcessor::createEditor()
{
    return new VocalFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void VocalFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void VocalFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Gain1], paramsNames[Parameters::Gain1], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Gain2], paramsNames[Parameters::Gain2], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Gain3], paramsNames[Parameters::Gain3], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Gain4], paramsNames[Parameters::Gain4], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Gain5], paramsNames[Parameters::Gain5], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume], paramsNames[Parameters::Volume], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalFilterAudioProcessor();
}
