/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <juce_core/juce_core.h>

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

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("OS"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("PC"));
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
	// Initialize  oversampling
	oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
		getTotalNumOutputChannels(),					// number of channels
		OVERSAMPLING_FACTOR,							// oversampling factor. E.g.2^4 = 16x
		juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
		true);											// use interpolation for better quality

	oversampler->reset();
	oversampler->initProcessing(samplesPerBlock);

	setLatencySamples((int)oversampler->getLatencyInSamples());
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
	const auto type = static_cast<ClipperType>((int)typeParameter->load());
	const auto threshold = juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto wet = 0.01f * mixParameter->load();
	const auto oversample = button1Parameter->get();
	const auto postClip = button2Parameter->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto guiIsOpen = m_guiIsOpen.load();
	const int samples = buffer.getNumSamples();

	// Get input peak
	if (guiIsOpen)
	{
		m_inputMax = Math::fmaxf(std::fabsf(buffer.getMagnitude(0, samples)), m_inputMax);
	}

	if (oversample)
	{
		// Upsample
		juce::dsp::AudioBlock<float> block(buffer);
		auto oversampledBlock = oversampler->processSamplesUp(block);

		for (int channel = 0; channel < channels; channel++)
		{
			const Clippers::Params params{ threshold, wet, oversampledBlock.getChannelPointer(channel), oversampledBlock.getNumSamples() };
			clip(params, type, channel, (int)getSampleRate() * OVERSAMPLING_MULTIPLIER);
		}

		// Downsample
		oversampler->processSamplesDown(block);

		// Post clip
		if (postClip)
		{
			for (int channel = 0; channel < channels; channel++)
			{
				const Clippers::Params params{ threshold, wet, buffer.getWritePointer(channel), samples };
				Clippers::HardBlock(params);
			};
		}
	}
	else
	{
		for (int channel = 0; channel < channels; channel++)
		{
			const Clippers::Params params{ threshold, wet, buffer.getWritePointer(channel), samples };
			clip(params, type, channel, (int)getSampleRate());
		}
	}
	
	// Get output peak
	if (guiIsOpen)
	{
		m_outputMax = Math::fmaxf(std::fabsf(buffer.getMagnitude(0, samples)), m_outputMax);
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(volumeParameter->load()));
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

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   7.0f,  1.0f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -60.0f,   0.0f,  1.0f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("OS", "OS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("PC", "PC", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ClipperAudioProcessor();
}
