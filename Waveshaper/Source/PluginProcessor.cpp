/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================
#define PROCESS_WAVESHAPER(WAVESHAPER)												\
    for (int sample = 0; sample < samples; sample++) {                              \
        float& in = channelBuffer[sample];                                          \
        const float out = WAVESHAPER(gain * in, drive);                             \
        in = volume * ((1.0f - mix) * in + mix * out);                              \
    }

//==============================================================================

const std::string WaveshaperAudioProcessor::paramsNames[] = { "Type", "Gain", "Drive", "Mix", "Volume" };
const std::string WaveshaperAudioProcessor::paramsUnitNames[] = { "", " dB", "", " %", "dB" };

//==============================================================================
WaveshaperAudioProcessor::WaveshaperAudioProcessor()
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
	typeParameter   = apvts.getRawParameterValue(paramsNames[0]);
	gainParameter   = apvts.getRawParameterValue(paramsNames[1]);
	driveParameter  = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter    = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

WaveshaperAudioProcessor::~WaveshaperAudioProcessor()
{
}

//==============================================================================
const juce::String WaveshaperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WaveshaperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WaveshaperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WaveshaperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WaveshaperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WaveshaperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WaveshaperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WaveshaperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WaveshaperAudioProcessor::getProgramName (int index)
{
    return {};
}

void WaveshaperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WaveshaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
}

void WaveshaperAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WaveshaperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void WaveshaperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto type   = typeParameter->load();
	const auto gain   = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto drive  = 1.0f + (7.0f / 100.0f) * driveParameter->load();
	const auto mix    = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		if (type == 1)
		{
			PROCESS_WAVESHAPER(Waveshapers::Tanh)
		}
		else if(type == 2)
		{
			PROCESS_WAVESHAPER(Waveshapers::Reciprocal)
		}
		else if (type == 3)
		{
			PROCESS_WAVESHAPER(Waveshapers::Atan)
		}
		else if (type == 4)
		{
			PROCESS_WAVESHAPER(Waveshapers::Sin)
		}
		else if (type == 5)
		{
			PROCESS_WAVESHAPER(Waveshapers::ARRY)
		}
		else if (type == 6)
		{
			PROCESS_WAVESHAPER(Waveshapers::Linear)
		}
		else if (type == 7)
		{
			PROCESS_WAVESHAPER(Waveshapers::Quadratic)
		}
	}
}

//==============================================================================
bool WaveshaperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WaveshaperAudioProcessor::createEditor()
{
    return new WaveshaperAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void WaveshaperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void WaveshaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout WaveshaperAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   7.0f,  1.0f, 1.0f),  1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveshaperAudioProcessor();
}
