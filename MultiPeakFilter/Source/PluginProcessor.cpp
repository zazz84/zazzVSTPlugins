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

const std::string MultiPeakFilterAudioProcessor::paramsNames[] =		{ "Frequency",	"Q",	"Gain",	"Step", "Count",	"Slope",	"Volume" };
const std::string MultiPeakFilterAudioProcessor::labelNames[] =		{ "Frequency",	"Q",	"Gain",	"Step", "Count",	"Slope",	"Volume" };
const std::string MultiPeakFilterAudioProcessor::paramsUnitNames[] = { " Hz",		"",		" dB",	" st",	"",			"",			" dB" };

//==============================================================================
MultiPeakFilterAudioProcessor::MultiPeakFilterAudioProcessor()
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
	frequencyParameter	= apvts.getRawParameterValue(paramsNames[0]);
	qParameter			= apvts.getRawParameterValue(paramsNames[1]);
	gainParameter		= apvts.getRawParameterValue(paramsNames[2]);
	stepParameter		= apvts.getRawParameterValue(paramsNames[3]);
	countParameter		= apvts.getRawParameterValue(paramsNames[4]);
	slopeParameter		= apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[6]);
}

MultiPeakFilterAudioProcessor::~MultiPeakFilterAudioProcessor()
{
}

//==============================================================================
const juce::String MultiPeakFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiPeakFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiPeakFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiPeakFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiPeakFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiPeakFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiPeakFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiPeakFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiPeakFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiPeakFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiPeakFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		for (int filter = 0; filter < COUNT_MAX; filter++)
		{
			m_filter[channel][filter].init(sr);
		}
	}

	m_frequencySmoother[0].init(sr);
	m_frequencySmoother[1].init(sr);
	m_qSmoother[0].init(sr);
	m_qSmoother[1].init(sr);
	m_gainSmoother[0].init(sr);
	m_gainSmoother[1].init(sr);

	constexpr float frequency = 3.0f;

	m_frequencySmoother[0].set(frequency);
	m_frequencySmoother[1].set(frequency);
	m_qSmoother[0].set(frequency);
	m_qSmoother[1].set(frequency);
	m_gainSmoother[0].set(frequency);
	m_gainSmoother[1].set(frequency);
	m_stepSmoother[0].set(frequency);
	m_stepSmoother[1].set(frequency);
	m_slopeSmoother[0].set(frequency);
	m_slopeSmoother[1].set(frequency);
}

void MultiPeakFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiPeakFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MultiPeakFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Prevent denormals for this function scope
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto frequency = frequencyParameter->load();
	const auto q = qParameter->load();
	const auto filterGain = gainParameter->load();
	const auto step = stepParameter->load();
	const auto count = countParameter->load();
	const auto slope = 0.01f * slopeParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto srHalf = 0.5f * static_cast<float>(getSampleRate());

	// Limit count to half of sampling freq
	// TODO: Solve better
	int countLimited = 0;

	for (int i = 0; i < count; i++)
	{
		if (Math::shiftFrequency(frequency, i * step) < 16000.0f)
		{
			countLimited = i;
		}
	}

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& frequencySmoother = m_frequencySmoother[channel];
		auto& qSmoother = m_qSmoother[channel];
		auto& gainSmoother = m_gainSmoother[channel];
		auto& stepSmoother = m_stepSmoother[channel];
		auto& slopeSmoother = m_slopeSmoother[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			float out = channelBuffer[sample];

			// Set filter
			const auto frequencySmooth = frequencySmoother.process(frequency);
			const auto qSmooth = qSmoother.process(q);
			const auto gainSmooth = gainSmoother.process(filterGain);
			const auto stepSmooth = stepSmoother.process(step);
			const auto slopeSmooth = slopeSmoother.process(slope);

			const auto gainStep = (1.0f - slopeSmooth) * (gainSmooth / (float)countLimited);

			// Set filters
			for (int i = 0; i <= countLimited; i++)
			{
				const float f = Math::shiftFrequency(frequencySmooth, i * stepSmooth);
				const float g = gainSmooth + i * gainStep;

				m_filter[channel][i].setPeak(f, qSmooth, g);

				// Process
				out = m_filter[channel][i].processDF1(out);
			}
		
			//Out
			channelBuffer[sample] = out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool MultiPeakFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiPeakFilterAudioProcessor::createEditor()
{
    return new MultiPeakFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MultiPeakFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MultiPeakFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiPeakFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  40.0f,   8000.0f,  1.0f, 0.4f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.3f,     24.0f,  0.1f, 0.7f),   8.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -36.0f,     36.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   1.0f,     48.0f, 0.01f, 1.0f),  12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   1.0f, COUNT_MAX,  1.0f, 1.0f),   4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,    200.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -36.0f,     36.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiPeakFilterAudioProcessor();
}
