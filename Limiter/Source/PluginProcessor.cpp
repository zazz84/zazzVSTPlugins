/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string LimiterAudioProcessor::paramsNames[] = { "Attack", "Release", "Threshold", "Volume" };
const std::string LimiterAudioProcessor::paramsUnitNames[] = { " ms", " ms", " dB", " dB" };

//==============================================================================
LimiterAudioProcessor::LimiterAudioProcessor()
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
	attackParameter    = apvts.getRawParameterValue(paramsNames[0]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[1]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);
}

LimiterAudioProcessor::~LimiterAudioProcessor()
{
}

//==============================================================================
const juce::String LimiterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LimiterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LimiterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LimiterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LimiterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LimiterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LimiterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LimiterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LimiterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sizeAttack = (int)(10.0 * 0.001 * sampleRate);

	m_buffer[0].init(sizeAttack);
	m_buffer[1].init(sizeAttack);

	const int sizeRelease = (int)(50.0 * 0.001 * sampleRate);

	m_samplesToPeak[0] = -sizeRelease;
	m_samplesToPeak[1] = -sizeRelease;

	m_currentPeak[0] = 0.0f;
	m_currentPeak[1] = 0.0f;

	m_interpolationMultiplier[0] = 1.0f;
	m_interpolationMultiplier[1] = 1.0f;
	
	m_interpolationSpeed[0] = 0.0f;
	m_interpolationSpeed[1] = 0.0f;
}

void LimiterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LimiterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LimiterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();
	const auto threshold = juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sizeAttack = (int)((double)attack * 0.001 * getSampleRate());
	const auto sizeRelease = (int)((double)release * 0.001 * getSampleRate());

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//References
		auto& delayBuffer = m_buffer[channel];
		auto& samplesToPeak = m_samplesToPeak[channel];
		auto& currentPeak = m_currentPeak[channel];
		auto& interpolationMultiplier = m_interpolationMultiplier[channel];
		auto& interpolationSpeed = m_interpolationSpeed[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			// In
			const float in = channelBuffer[sample];
			const float inDelayed = delayBuffer.readDelay(sizeAttack);
			delayBuffer.writeSample(in);

			const float inAbs = std::fabsf(in);
			if (inAbs > currentPeak)
			{
				if (inAbs > threshold)
				{
					const float belowThreshold = threshold - (inAbs - threshold);
					//const float toInterpolate = std::fabsf(inDelayed) - belowThreshold;
					interpolationSpeed = -belowThreshold / sizeAttack;
					currentPeak = inAbs;
					samplesToPeak = sizeAttack;
				}
				else
				{
					currentPeak = inAbs;
				}
			}

			if (samplesToPeak == 0)
			{
				interpolationSpeed = (currentPeak - threshold) / sizeRelease;
				currentPeak = threshold;
			}

			samplesToPeak--;

			if (samplesToPeak < -sizeRelease)
			{
				interpolationMultiplier = 1.0f;
			}
			else
			{
				interpolationMultiplier += interpolationSpeed;
			}

			if (interpolationMultiplier < 0.0f)
			{
				float test = -1.0f;
			}

			//Out
			channelBuffer[sample] = interpolationMultiplier * inDelayed;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool LimiterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LimiterAudioProcessor::createEditor()
{
    return new LimiterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void LimiterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void LimiterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout LimiterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f, 10.0f,  1.0f, 1.0f),  5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  10.0f, 50.0f,  1.0f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -30.0f,  0.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,  18.0f, 1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LimiterAudioProcessor();
}
