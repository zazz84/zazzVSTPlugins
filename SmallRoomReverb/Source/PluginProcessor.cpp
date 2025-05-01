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

const std::string SmallRoomReverbAudioProcessor::paramsNames[] =		{	"ER Predelay",	"ER Size", "ER Damping", "ER Width",	"LR Predelay", "LR Size",	"LR Damping",	"LR Width", "ER Volume",	"LR Volume",	"Mix",	"Volume" };
const std::string SmallRoomReverbAudioProcessor::paramsUnitNames[] =	{	" ms",			"",			"",			" %",			" ms",			"",			"",				" %",		" dB",			" dB",			" %",	" dB" };

//const float SmallRoomReverbAudioProcessor::ALLPASS_DELAY_TIMES_MS[] = { 5.4f, 17.3f, 23.0f, 32.8f };
//const float SmallRoomReverbAudioProcessor::COMP_DELAY_TIMES_MS[] = { 42.9f, 65.8f, 82.8f, 101.0f };

//const float SmallRoomReverbAudioProcessor::ALLPASS_DELAY_TIMES_MS[] = { 7.9f, 14.7f, 23.0f, 33.0f };
const float SmallRoomReverbAudioProcessor::ALLPASS_DELAY_TIMES_MS[] = {7.9, 14.7, 19.0, 23.0, 27.0, 33.0};
//const float SmallRoomReverbAudioProcessor::ALLPASS_DELAY_TIMES_MS[] = {15.8, 29.4, 38.0, 46.0, 54.0, 66.0};
const float SmallRoomReverbAudioProcessor::COMP_DELAY_TIMES_MS[] = { 20.6f, 57.3f, 71.1f, 100.9f };

const float SmallRoomReverbAudioProcessor::ALLPASS_DELAY_WIDTH[3][6] = {
	{ 0.9f, 0.0f, 0.9f, 0.0f, 0.9f, 0.0f },  // From ALLPASS_DELAY_WIDTH_1
	{ 0.0f, 0.9f, 0.0f, 0.9f, 0.0f, 0.9f },  // From ALLPASS_DELAY_WIDTH_2
	{ 0.6f, 0.1f, 0.6f, 0.1f, 0.6f, 0.1f }   // From ALLPASS_DELAY_WIDTH_3
};

//==============================================================================
SmallRoomReverbAudioProcessor::SmallRoomReverbAudioProcessor()
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
	ERpredelayParameter = apvts.getRawParameterValue(paramsNames[0]);
	ERsizeParameter = apvts.getRawParameterValue(paramsNames[1]);
	ERdampingParameter = apvts.getRawParameterValue(paramsNames[2]);
	ERwidthParameter = apvts.getRawParameterValue(paramsNames[3]);

	LRpredelayParameter = apvts.getRawParameterValue(paramsNames[4]);
	LRsizeParameter = apvts.getRawParameterValue(paramsNames[5]);
	LRdampingParameter = apvts.getRawParameterValue(paramsNames[6]);
	LRwidthParameter = apvts.getRawParameterValue(paramsNames[7]);

	ERvolumeParameter = apvts.getRawParameterValue(paramsNames[8]);
	LRvolumeParameter = apvts.getRawParameterValue(paramsNames[9]);
	mixParameter = apvts.getRawParameterValue(paramsNames[10]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[11]);
}

SmallRoomReverbAudioProcessor::~SmallRoomReverbAudioProcessor()
{
}

//==============================================================================
const juce::String SmallRoomReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SmallRoomReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SmallRoomReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SmallRoomReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SmallRoomReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SmallRoomReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SmallRoomReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SmallRoomReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void SmallRoomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SmallRoomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const float sampleRateMS = 0.001f * sampleRate;

	for (int channelReverb = 0; channelReverb < MAX_CHANNELS; channelReverb++)
	{
		for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
		{
			const int size = BUFFER_MINIMUM_SIZE + (int)(ALLPASS_DELAY_TIMES_MS[allpass] * sampleRateMS);
			m_allpass[channelReverb][allpass].init(size);
		}

		for (int comb = 0; comb < N_COMBS; comb++)
		{
			const int size = BUFFER_MINIMUM_SIZE + (int)(COMP_DELAY_TIMES_MS[comb] * sampleRateMS);
			m_combs[channelReverb][comb].init(size);
		}
	}

	const int sr = (int)sampleRate;

	m_filter[0].init(sr);
	m_filter[1].init(sr);

	const int size = (int)(sampleRate * 0.001 * (10.0 + 60.0 + 30.0));

	m_earlyReflections[0].init(size, 0);
	m_earlyReflections[1].init(size, 1);
}

void SmallRoomReverbAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SmallRoomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SmallRoomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto ERpredelay = ERpredelayParameter->load();
	const auto ERsize = 0.2f + 0.8f * ERsizeParameter->load();
	const auto ERdamping = sqrtf(ERdampingParameter->load());
	auto ERwidth = 0.01f * ERwidthParameter->load();
	ERwidth = ERwidth * ERwidth * ERwidth;

	const auto LRpredelay = LRpredelayParameter->load();
	const auto LRsize = LRsizeParameter->load();
	const auto LRdamping = sqrtf(LRdampingParameter->load());
	auto LRwidth = 0.01f *LRwidthParameter->load();
	LRwidth = LRwidth * LRwidth * LRwidth;

	const auto ERgain = juce::Decibels::decibelsToGain(ERvolumeParameter->load());
	const auto LRgain = juce::Decibels::decibelsToGain(LRvolumeParameter->load() + 3.0f);		// Added output volume conpensation
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRateMS = 0.001f * (float)getSampleRate();

	const auto ERpredelaySize = (int)(sampleRateMS * ERpredelay);
	const auto ERsizeSamples = (int)(sampleRateMS * ERsize * 60.0f);
	const auto ERDampingFrequency = 16000.0f - ERdamping * 14000.0f;
	const auto ERDampingQ = 0.707f - ERdamping * 0.207;

	const auto LRPredelaySize = 1 + (int)(sampleRateMS * LRpredelay);

	// Set late reflections
	const auto APReflectionsTime = 0.2f + 0.6f * LRsize;
	const auto CFReflectionsTime = 0.2f + 0.4f * LRsize;
	const auto CFReflectionsDensity = 0.28f + 0.43f * LRsize;

	// Update
	const int channelsReverb = (channels < MAX_CHANNELS) ? channels : MAX_CHANNELS;

	for (int channelReverb = 0; channelReverb < channelsReverb; channelReverb++)
	{
		for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
		{
			const int delay = BUFFER_MINIMUM_SIZE + static_cast<int>(APReflectionsTime * ALLPASS_DELAY_TIMES_MS[allpass] * sampleRateMS * (1.0f - ALLPASS_DELAY_WIDTH[channelReverb][allpass] * LRwidth));
			m_allpass[channelReverb][allpass].set(delay);
		}

		for (int comb = 0; comb < N_COMBS; comb++)
		{
			const int delay = BUFFER_MINIMUM_SIZE + static_cast<int>(CFReflectionsTime * COMP_DELAY_TIMES_MS[comb] * sampleRateMS);
			m_combs[channelReverb][comb].set(delay);
			m_combs[channelReverb][comb].setGain(CFReflectionsDensity);
			m_combs[channelReverb][comb].setDamping(LRdamping);
		}
	}

	// Process
	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& filter = m_filter[channel];
		auto& earlyReflections = m_earlyReflections[channel];

		filter.setLowPass(ERDampingFrequency, ERDampingQ);
		earlyReflections.set(ERpredelaySize, ERsizeSamples, ERwidth);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			// ER Damping
			float filterOut = filter.processDF1(in);

			// Process early reflections
			const float ERout = earlyReflections.process(filterOut);

			// Process serial allpass filters
			float allPassOut = earlyReflections.readDelay(LRPredelaySize);

			for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
			{
				allPassOut = m_allpass[channel][allpass].process(allPassOut);
			}

			/*float LROut = 0.0f;
			// Process paraler comb filters		
			for (int comb = 0; comb < N_COMBS; comb++)
			{
				LROut += m_combs[channel][comb].process(allPassOut);
			}*/

			float LROut = allPassOut;

			//Out
			const float out = ERgain * ERout + LRgain * LROut;
			channelBuffer[sample] = in - mix * (in - out);
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool SmallRoomReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SmallRoomReverbAudioProcessor::createEditor()
{
    return new SmallRoomReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SmallRoomReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SmallRoomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SmallRoomReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f,  10.0f,  0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f,  1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f, 100.0f,  0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f,   1.0f,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f, 100.0f,  1.0f,  1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>( -60.0f,   0.0f,  0.1f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>( -18.0f,  18.0f,  0.1f,  1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmallRoomReverbAudioProcessor();
}
