/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================
#define PROCESS_WAVESHAPER(WAVESHAPER)												\
    for (int sample = 0; sample < samples; sample++) {                              \
        float& in = channelBuffer[sample];                                          \
		float out = preFilter.processDF1(gain * in);                                \
        out = WAVESHAPER(out, drive, asymetry);										\
		out = postFilter.processDF1(out);											\
        in = volume * ((1.0f - mix) * in + mix * out);								\
    }

#define PROCESS_WAVESHAPER_SPLIT(WAVESHAPER)										\
    for (int sample = 0; sample < samples; sample++) {                              \
        float& in = channelBuffer[sample];                                          \
		float out = Waveshapers::Split2(in, splitThreshold, split);			        \
		out = preFilter.processDF1(gain * out);										\
        out = WAVESHAPER(out, drive, asymetry);										\
		out = postFilter.processDF1(out);											\
        in = volume * ((1.0f - mix) * in + mix * out);								\
    }

//==============================================================================

const std::string WaveshaperAudioProcessor::paramsNames[] = { "Type", "Gain", "Color", "Split", "Asymetry", "Mix", "Volume" };
const std::string WaveshaperAudioProcessor::labelNames[]  = { "Type", "Gain", "Color", "Split", "Asymetry", "Mix", "Volume" };
const std::string WaveshaperAudioProcessor::paramsUnitNames[] = { "", " dB", "", " dB", " %", " %", "dB" };

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
	typeParameter		= apvts.getRawParameterValue(paramsNames[0]);
	gainParameter		= apvts.getRawParameterValue(paramsNames[1]);
	colorParameter		= apvts.getRawParameterValue(paramsNames[2]);
	splitParameter		= apvts.getRawParameterValue(paramsNames[3]);
	asymetryParameter	= apvts.getRawParameterValue(paramsNames[4]);
	mixParameter		= apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[6]);
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

	m_preFilter[0].init(sr);
	m_preFilter[1].init(sr);

	m_postFilter[0].init(sr);
	m_postFilter[1].init(sr);
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
	const auto gain = type != 3 ? juce::Decibels::decibelsToGain(gainParameter->load()) : juce::Decibels::decibelsToGain(gainParameter->load() - 6.0f);
	const auto color  = 18.0f * 0.01f * colorParameter->load();
	const auto splitdB  = splitParameter->load();
	const auto asymetry  = 0.01f * asymetryParameter->load();
	const auto mix    = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto split = juce::Decibels::decibelsToGain(splitdB);
	const auto splitThreshold = juce::Decibels::decibelsToGain(-40.0f);

	if (splitdB < -39.5f)
	{
		for (int channel = 0; channel < channels; ++channel)
		{
			// Filter
			auto& preFilter = m_preFilter[channel];
			auto& postFilter = m_postFilter[channel];

			constexpr float frequency = 440.0f;

			if (color < 0.0f)
			{
				preFilter.setLowShelf(frequency, 0.707f, 0.5f * color);
				postFilter.setLowShelf(frequency, 0.707f, -0.5f * color);
			}
			else
			{
				preFilter.setLowShelf(frequency, 0.707f, color);
				postFilter.setLowShelf(frequency, 0.707f, -color);
			}

			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);
			if (type == 1)
			{
				const auto drive = 1.0f;

				PROCESS_WAVESHAPER(Waveshapers::Tanh)
			}
			else if (type == 2)
			{
				const auto drive = 1.0f;

				PROCESS_WAVESHAPER(Waveshapers::Reciprocal)
			}
			else if (type == 3)
			{
				const auto drive = Math::remap(gain, 0.0f, 18.0f, 1.0f, 8.0f);

				PROCESS_WAVESHAPER(Waveshapers::Exponential)
			}
		}
	}
	else
	{
		for (int channel = 0; channel < channels; ++channel)
		{
			// Filter
			auto& preFilter = m_preFilter[channel];
			auto& postFilter = m_postFilter[channel];

			constexpr float frequency = 440.0f;

			if (color < 0.0f)
			{
				preFilter.setLowShelf(frequency, 0.707f, 0.5f * color);
				postFilter.setLowShelf(frequency, 0.707f, -0.5f * color);
			}
			else
			{
				preFilter.setLowShelf(frequency, 0.707f, color);
				postFilter.setLowShelf(frequency, 0.707f, -color);
			}

			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);
			if (type == 1)
			{				
				const auto drive = 1.0f;

				PROCESS_WAVESHAPER_SPLIT(Waveshapers::Tanh)
			}
			else if (type == 2)
			{
				const auto drive = 1.0f;

				PROCESS_WAVESHAPER_SPLIT(Waveshapers::Reciprocal)
			}
			else if (type == 3)
			{
				const auto drive = Math::remap(gain, 0.0f, 18.0f, 1.0f, 8.0f);

				PROCESS_WAVESHAPER_SPLIT(Waveshapers::Exponential)
			}
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

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(    1.0f,   3.0f,  1.0f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  -36.0f,  36.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -100.0f, 100.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  -40.0f,   0.0f,  1.0f, 1.0f), -40.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -100.0f, 100.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(    0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(  -36.0f,  36.0f,  1.0f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveshaperAudioProcessor();
}
