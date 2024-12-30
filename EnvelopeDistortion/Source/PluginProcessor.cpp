/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string EnvelopeDistortionAudioProcessor::paramsNames[] = { "Type", "Frequency", "Gain", "Release", "Mix", "Volume" };
const std::string EnvelopeDistortionAudioProcessor::paramsUnitNames[] = { "", " Hz", " dB", " %", " %", " dB" };

//==============================================================================
EnvelopeDistortionAudioProcessor::EnvelopeDistortionAudioProcessor()
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
	typeParameter		= apvts.getRawParameterValue(paramsNames[0]);
	frequencyParameter	= apvts.getRawParameterValue(paramsNames[1]);
	gainParameter		= apvts.getRawParameterValue(paramsNames[2]);
	releaseParameter    = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter		= apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[5]);
}

EnvelopeDistortionAudioProcessor::~EnvelopeDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String EnvelopeDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnvelopeDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EnvelopeDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EnvelopeDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EnvelopeDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EnvelopeDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EnvelopeDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnvelopeDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EnvelopeDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnvelopeDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EnvelopeDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	
	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_enveloepFollower[channel].init(sr);
		m_dcFilter[channel].init(sr);
		m_dcFilter[channel].set(30.0f);

		m_preEmphasisFilter[channel].init(sr);
		m_deEmphasisFilter[channel].init(sr);
	}
}

void EnvelopeDistortionAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnvelopeDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EnvelopeDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{	
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto type = static_cast<int>(typeParameter->load());
	const auto frequency = frequencyParameter->load();
	const auto filterGain = gainParameter->load();
	const auto release = 0.001 + 0.09999 * static_cast<double>(releaseParameter->load());
	const auto mix = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto wet = gain * mix;
	const auto dry = gain * (1.0f - mix);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& envelopeFollower = m_enveloepFollower[channel];
		auto& dcFilter = m_dcFilter[channel];
		auto& preEmphasisFilter = m_preEmphasisFilter[channel];
		auto& deEmphasisFilter = m_deEmphasisFilter[channel];

		envelopeFollower.set(0.0, release);
		preEmphasisFilter.setPeak(frequency, 0.5f, filterGain);
		deEmphasisFilter.setPeak(frequency, 0.5f, -filterGain);

		if(type == 1)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Preemphasis
				float out = preEmphasisFilter.processDF1(in);
				
				// Type 1
				out = std::max(0.0f, out);

				// Envelope
				out = static_cast<float>(envelopeFollower.process(static_cast<double>(out)));

				// DC filter
				out = dcFilter.process(out);

				// Deemphasis
				out = deEmphasisFilter.processDF1(out);

				//Out
				channelBuffer[sample] = dry * in + wet * out;
			}
		}
		else if (type == 2)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Preemphasis
				float out = preEmphasisFilter.processDF1(in);

				// Type 2
				out = std::abs(out);

				// Envelope
				out = static_cast<float>(envelopeFollower.process(static_cast<double>(out)));

				// DC filter
				out = dcFilter.process(out);

				// Deemphasis
				out = deEmphasisFilter.processDF1(out);

				//Out
				channelBuffer[sample] = dry * in + wet * out;
			}
		}
		else
		{
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Preemphasis
				float out = preEmphasisFilter.processDF1(in);

				// Type 3
				out = std::max(0.0f, out + 1.0f);

				// Envelope
				out = static_cast<float>(envelopeFollower.process(static_cast<double>(out)));

				// DC filter
				out = dcFilter.process(out);

				// Deemphasis
				out = deEmphasisFilter.processDF1(out);

				//Out
				channelBuffer[sample] = dry * in + wet * out;
			}
		}
	}
}

//==============================================================================
bool EnvelopeDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EnvelopeDistortionAudioProcessor::createEditor()
{
    return new EnvelopeDistortionAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void EnvelopeDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void EnvelopeDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout EnvelopeDistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 1.0f, 3.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 40.0f, 10000.0f, 1.0f, 0.4f), 6000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 40.0f, 10000.0f, 1.0f, 0.4f), 6000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -24.0f, 24.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 0.0f, 100.0f, 0.01f, 0.3f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnvelopeDistortionAudioProcessor();
}
