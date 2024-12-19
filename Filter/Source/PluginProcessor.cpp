/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string FilterAudioProcessor::paramsNames[] = {		"HP Slope",		"HP Frequency",
																"LP Slope",		"LP Frequency",
																"Mix",			"Volume" };
const std::string FilterAudioProcessor::paramsUnitNames[] = {	" dB/oct",		" Hz",
																" dB/oct",		" Hz",
																" %",			" dB" };

//==============================================================================
FilterAudioProcessor::FilterAudioProcessor()
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
	HPSlopeParameter     = apvts.getRawParameterValue(paramsNames[0]);
	HPFrequencyParameter = apvts.getRawParameterValue(paramsNames[1]);
	LPSlopeParameter     = apvts.getRawParameterValue(paramsNames[2]);
	LPFrequencyParameter = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter         = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter      = apvts.getRawParameterValue(paramsNames[5]);
}

FilterAudioProcessor::~FilterAudioProcessor()
{
}

//==============================================================================
const juce::String FilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void FilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_LowPass8[0].init(sr);
	m_LowPass8[1].init(sr);
	m_HighPass8[0].init(sr);
	m_HighPass8[1].init(sr);

	m_LowPass6[0].init(sr);
	m_LowPass6[1].init(sr);
	m_HighPass6[0].init(sr);
	m_HighPass6[1].init(sr);

	m_LowPass4[0].init(sr);
	m_LowPass4[1].init(sr);
	m_HighPass4[0].init(sr);
	m_HighPass4[1].init(sr);

	m_LowPass2[0].init(sr);
	m_LowPass2[1].init(sr);
	m_HighPass2[0].init(sr);
	m_HighPass2[1].init(sr);
}

void FilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto HPSlope = HPSlopeParameter->load();
	const auto HPFrequency = HPFrequencyParameter->load();
	const auto LPSlope = LPSlopeParameter->load();
	const auto LPFrequency = LPFrequencyParameter->load();
	const auto wet = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto dry = 1.0f - wet;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// Set frequency
		if (HPSlope == 12.0f)
		{
			m_HighPass2[channel].setHighPass(HPFrequency, 0.710f);
		}
		else if ((HPSlope == 24.0f))
		{
			m_HighPass4[channel].set(HPFrequency);
		}
		else if ((HPSlope == 36.0f))
		{
			m_HighPass6[channel].set(HPFrequency);
		}
		else
		{
			m_HighPass8[channel].set(HPFrequency);
		}

		// Low pass filter
		if (LPSlope == 12.0f)
		{
			m_LowPass2[channel].setLowPass(LPFrequency, 0.710f);
		}
		else if ((LPSlope == 24.0f))
		{
			m_LowPass4[channel].set(LPFrequency);
		}
		else if ((LPSlope == 36.0f))
		{
			m_LowPass6[channel].set(LPFrequency);
		}
		else
		{
			m_LowPass8[channel].set(LPFrequency);
		}

		for (int sample = 0; sample < samples; sample++)
		{
			// Input
			const float in = channelBuffer[sample];

			// High pass filter
			float out = 0.0f;
			if (HPSlope == 12.0f)
			{
				out = m_HighPass2[channel].processDF1(in);
			}
			else if ((HPSlope == 24.0f))
			{
				out = m_HighPass4[channel].process(in);
			}
			else if ((HPSlope == 36.0f))
			{
				out = m_HighPass6[channel].process(in);
			}
			else
			{
				out = m_HighPass8[channel].process(in);
			}

			// Low pass filter
			if (LPSlope == 12.0f)
			{
				out = m_LowPass2[channel].processDF1(out);
			}
			else if ((LPSlope == 24.0f))
			{
				out = m_LowPass4[channel].process(out);
			}
			else if ((LPSlope == 36.0f))
			{
				out = m_LowPass6[channel].process(out);
			}
			else
			{
				out = m_LowPass8[channel].process(out);
			}
			
			// Output
			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool FilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FilterAudioProcessor::createEditor()
{
    return new FilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void FilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void FilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout FilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 12.0f, 48.0f, 12.0f, 1.0f), 12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 10.0f, 22000.0f, 2.0f, 0.4f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 12.0f, 48.0f, 12.0f, 1.0f), 12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 10.0f, 22000.0f, 2.0f, 0.4f), 22000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FilterAudioProcessor();
}
