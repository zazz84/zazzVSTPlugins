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

const std::string VelvetNoiseReverbAudioProcessor::paramsNames[] = { "Predelay", "Decay", "Shape", "Density", "Low", "High", "Width", "Mix", "Volume" };
const std::string VelvetNoiseReverbAudioProcessor::paramsUnitNames[] = { " ms", " s", "", " %", " %", " %", " %", " %", " dB" };

//==============================================================================
VelvetNoiseReverbAudioProcessor::VelvetNoiseReverbAudioProcessor()
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
	preDelayTimeParameter	= apvts.getRawParameterValue(paramsNames[0]);
	reverbTimeParameter		= apvts.getRawParameterValue(paramsNames[1]);
	decayShapeParameter		= apvts.getRawParameterValue(paramsNames[2]);
	densityParameter		= apvts.getRawParameterValue(paramsNames[3]);
	lowParameter			= apvts.getRawParameterValue(paramsNames[4]);
	highParameter			= apvts.getRawParameterValue(paramsNames[5]);
	widthParameter			= apvts.getRawParameterValue(paramsNames[6]);
	mixParameter			= apvts.getRawParameterValue(paramsNames[7]);
	volumeParameter			= apvts.getRawParameterValue(paramsNames[8]);
}

VelvetNoiseReverbAudioProcessor::~VelvetNoiseReverbAudioProcessor()
{
}

//==============================================================================
const juce::String VelvetNoiseReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VelvetNoiseReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VelvetNoiseReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VelvetNoiseReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VelvetNoiseReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VelvetNoiseReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VelvetNoiseReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VelvetNoiseReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VelvetNoiseReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void VelvetNoiseReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VelvetNoiseReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	const float lengthSeconds = 2.0f + 0.1f;

	m_reverb[0].init(sr, lengthSeconds);
	m_reverb[1].init(sr, lengthSeconds);
}

void VelvetNoiseReverbAudioProcessor::releaseResources()
{
	m_reverb[0].release();
	m_reverb[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VelvetNoiseReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VelvetNoiseReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Prevent denormals for this function scope
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto preDelayTime = 0.001f * preDelayTimeParameter->load();
	const auto reverbTime = reverbTimeParameter->load();
	const auto decayShape = decayShapeParameter->load();
	const auto density = 0.01f * densityParameter->load();
	const auto low = 0.01f * lowParameter->load();
	const auto high = 0.01f * highParameter->load();
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto reverbGainCompensation = juce::Decibels::decibelsToGain(-32.0f + (1.0f - density) * 9.0f);

	//TEMP
	m_reverb[0].set(reverbTime, preDelayTime, decayShape, density, 79L, low, high);
	m_reverb[1].set(reverbTime, preDelayTime, decayShape, density, 99L, low, high);

	if (channels == 1)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);
		auto& reverb = m_reverb[0];

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			const float out = reverbGainCompensation * reverb.process(in);

			//Out
			channelBuffer[sample] = (1.0f - mix) * in + mix * out;
		}
	}
	else if (channels == 2)
	{
		// Channel pointer
		auto* leftChannelBuffer = buffer.getWritePointer(0);
		auto* rightChannelBuffer = buffer.getWritePointer(1);

		auto& leftReverb = m_reverb[0];
		auto& rightReverb = m_reverb[1];

		// MidSide gain
		const auto width = 0.01f * widthParameter->load();
		const float midGain = 2.0f - width;
		const float sideGain = width;

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float inLeft = leftChannelBuffer[sample];
			const float inRight = rightChannelBuffer[sample];

			// Process reverb
			float outLeft = reverbGainCompensation * leftReverb.process(inLeft);
			float outRight = reverbGainCompensation * rightReverb.process(inRight);

			// Handle MS
			const float mid = midGain * (outLeft + outRight);
			const float side = sideGain * (outLeft - outRight);

			outLeft = mid + side;
			outRight = mid - side;

			//Out
			leftChannelBuffer[sample] = (1.0f - mix) * inLeft + mix * outLeft;
			rightChannelBuffer[sample] = (1.0f - mix) * inRight + mix * outRight;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool VelvetNoiseReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VelvetNoiseReverbAudioProcessor::createEditor()
{
    return new VelvetNoiseReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void VelvetNoiseReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void VelvetNoiseReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout VelvetNoiseReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.1f,   2.0f, 0.01f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.1f,   2.0f, 0.01f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f, 200.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VelvetNoiseReverbAudioProcessor();
}
