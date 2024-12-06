/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string TransientShaperAudioProcessor::paramsNames[] = { "Attack", "Sustain", "Volume" };
const std::string TransientShaperAudioProcessor::paramsUnitNames[] = { "", "", " dB" };

//==============================================================================
TransientShaperAudioProcessor::TransientShaperAudioProcessor()
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
	attackParameter      = apvts.getRawParameterValue(paramsNames[0]);
	sustainParameter     = apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter      = apvts.getRawParameterValue(paramsNames[2]);
}

TransientShaperAudioProcessor::~TransientShaperAudioProcessor()
{
}

//==============================================================================
const juce::String TransientShaperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TransientShaperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TransientShaperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TransientShaperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TransientShaperAudioProcessor::getTailLengthSeconds() const
{
    return 0.150;
}

int TransientShaperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TransientShaperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TransientShaperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TransientShaperAudioProcessor::getProgramName (int index)
{
    return {};
}

void TransientShaperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TransientShaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_envelopeFollowerSlow[0].init(sr);
	m_envelopeFollowerSlow[1].init(sr);

	m_envelopeFollowerFast[0].init(sr);
	m_envelopeFollowerFast[1].init(sr);

	constexpr float attackTimeSlow = 5.0f;
	constexpr float releaseTimeSlow = 150.0f;

	m_envelopeFollowerSlow[0].set(attackTimeSlow, releaseTimeSlow);
	m_envelopeFollowerSlow[1].set(attackTimeSlow, releaseTimeSlow);
	
	constexpr float attackTime = 0.3f;
	constexpr float releaseTime = 30.0f;

	m_envelopeFollowerFast[0].set(attackTime, releaseTime);
	m_envelopeFollowerFast[1].set(attackTime, releaseTime);
}

void TransientShaperAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TransientShaperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TransientShaperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto attackGain = 0.01f * attackParameter->load();
	const auto sustainGain = -0.01f * sustainParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// Envelope follower
		auto& envelopeFollowerSlow = m_envelopeFollowerSlow[channel];
		auto& envelopeFollowerFast = m_envelopeFollowerFast[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			float& in = channelBuffer[sample];
			const float envelopeIn = std::fabsf(in);
			const float envelopeSlow = envelopeFollowerSlow.process(envelopeIn);
			const float envelopeFast = envelopeFollowerFast.process(envelopeIn);

			float differencedB = juce::Decibels::gainToDecibels(envelopeFast) - juce::Decibels::gainToDecibels(envelopeSlow);
			const float gaindB = differencedB > 0.0f ? differencedB * attackGain : differencedB * sustainGain;

			in = volume * juce::Decibels::decibelsToGain(gaindB) * in;
		}
	}
}

//==============================================================================
bool TransientShaperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TransientShaperAudioProcessor::createEditor()
{
    return new TransientShaperAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void TransientShaperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TransientShaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout TransientShaperAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TransientShaperAudioProcessor();
}
