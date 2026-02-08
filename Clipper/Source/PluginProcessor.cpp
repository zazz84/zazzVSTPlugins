/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <juce_core/juce_core.h>

//==============================================================================
#define PROCESS_CLIPPER(CLIPPER)												    \
    for (int sample = 0; sample < samples; sample++){                               \
        float& in = channelBuffer[sample];                                          \
        const float out = CLIPPER(in, threshold);                                   \
        in = dry * in + wet * out;												\
    }

//==============================================================================

const std::string ClipperAudioProcessor::paramsNames[] = { "Type", "Threshold", "Mix", "Volume" };
const std::string ClipperAudioProcessor::labelNames[] =  { "Type", "Threshold", "Mix", "Volume" };
const std::string ClipperAudioProcessor::paramsUnitNames[] = { "", " dB", " %", " dB" };

//==============================================================================
ClipperAudioProcessor::ClipperAudioProcessor()
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
	typeParameter      = apvts.getRawParameterValue(paramsNames[0]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[1]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);
}

ClipperAudioProcessor::~ClipperAudioProcessor()
{
}

//==============================================================================
const juce::String ClipperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ClipperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ClipperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ClipperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ClipperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ClipperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ClipperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ClipperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ClipperAudioProcessor::getProgramName (int index)
{
    return {};
}

void ClipperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ClipperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_slopeClipper[0].init(sr);
	m_slopeClipper[1].init(sr);
}

void ClipperAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ClipperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ClipperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto type = typeParameter->load();
	const auto threshold = juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto thresholdLow = -threshold;
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());
	const auto temp = 0.01f * mixParameter->load();
	const auto wet = gain * temp;
	const auto dry = gain * (1.0f - temp);

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto guiIsOpen = m_guiIsOpen.load();

	// Get input maximum
	if (guiIsOpen)
	{
		const float inputMax = std::fabsf(buffer.getMagnitude(0, samples));
		if (inputMax > m_inputMax)
		{
			m_inputMax = inputMax;
		}
	}

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
				
		if (type == Type::Hard)
		{			
#if JUCE_USE_SSE_INTRINSICS
			Clippers::HardClipSSE(channelBuffer, samples, -threshold, threshold, dry, wet);
#else
			for (int sample = 0; sample < samples; ++sample)
			{
				float& in = channelBuffer[sample];
				const float a = in < thresholdLow ? thresholdLow : in;
				const float out = a > threshold ? threshold : a;
				in = dry * in + wet * out;
			}
#endif		
		}
		else if (type == Type::Slope)
		{
			auto& slopeClipper = m_slopeClipper[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				float& in = channelBuffer[sample];
				const float out = slopeClipper.process(in, threshold);
				in = dry * in + wet * out;
			}
		}
		else if (type == Type::Soft)
		{
			PROCESS_CLIPPER(Clippers::SoftClip)
		}
		else if (type == Type::FoldBack)
		{
			PROCESS_CLIPPER(Clippers::FoldBack)
		}
		else if (type == Type::HalfWay)
		{
#if JUCE_USE_SSE_INTRINSICS
			Clippers::HardClipSSE(channelBuffer, samples, 0.0f, threshold, dry, wet);
#else
			for (int sample = 0; sample < samples; ++sample)
			{
				float& in = channelBuffer[sample];
				const float a = in < 0.0f ? 0.0f : in;
				const float out = a > threshold ? threshold : a;
				in = dry * in + wet * out;
			}
#endif
		}
		else if (type == Type::ABS)
		{
			PROCESS_CLIPPER(Clippers::ABS)
		}
	}

	// Get output maximum
	if (guiIsOpen)
	{
		const float outputMax = std::fabsf(buffer.getMagnitude(0, samples));
		if (outputMax > m_outputMax)
		{
			m_outputMax = outputMax;
		}
	}
}

//==============================================================================
bool ClipperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ClipperAudioProcessor::createEditor()
{
    return new ClipperAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ClipperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ClipperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ClipperAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   6.0f,  1.0f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -60.0f,   0.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("OS", "OS", true));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipperAudioProcessor();
}
