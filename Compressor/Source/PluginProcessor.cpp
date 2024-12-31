/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
#define PROCESS_COMPRESSOR(COMPRESSOR) \
		auto& compressor = COMPRESSOR[channel]; \
		compressor.set(-20.0f, ratio, 0.0f, attack, release, peakRatio, logRatio); \
		for (int sample = 0; sample < samples; sample++) \
		{ \
			const float in = channelBuffer[sample]; \
			const float	out = compressor.processHardKnee(in * gain); \
			channelBuffer[sample] = dry * in + wet * out; \
		} \

//==============================================================================

const std::string CompressorAudioProcessor::paramsNames[] = { "Type", "Gain", "Attack", "Release", "Ratio", "Peak/RMS", "Log/Lin", "Mix", "Volume" };
const std::string CompressorAudioProcessor::paramsUnitNames[] = { "", " dB", " ms", " ms", "", " %", " %", " %", " dB" };

//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
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
	typeParameter    = apvts.getRawParameterValue(paramsNames[0]);
	gainParameter    = apvts.getRawParameterValue(paramsNames[1]);
	attackParameter  = apvts.getRawParameterValue(paramsNames[2]);
	releaseParameter = apvts.getRawParameterValue(paramsNames[3]);
	ratioParameter   = apvts.getRawParameterValue(paramsNames[4]);
	rmsParameter     = apvts.getRawParameterValue(paramsNames[5]);
	linParameter     = apvts.getRawParameterValue(paramsNames[6]);
	mixParameter     = apvts.getRawParameterValue(paramsNames[7]);
	volumeParameter  = apvts.getRawParameterValue(paramsNames[8]);
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const juce::String CompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorAudioProcessor::getTailLengthSeconds() const
{
    return 1.0f;
}

int CompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = int(sampleRate);

	m_compressor[0].init(sr);
	m_compressor[1].init(sr);
	m_slewCompressor[0].init(sr);
	m_slewCompressor[1].init(sr);
	m_optoCompressor[0].init(sr);
	m_optoCompressor[1].init(sr);
	m_dualCompressor[0].init(sr);
	m_dualCompressor[1].init(sr);
	m_adaptiveCompressor[0].init(sr);
	m_adaptiveCompressor[1].init(sr);
}

void CompressorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto type = static_cast<int>(typeParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();
	const auto ratio = ratioParameter->load();
	const auto peakRatio = 1.0f - (0.01f * rmsParameter->load());
	const auto logRatio = 1.0f - (0.01f * linParameter->load());
	const auto mix = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto wet = volume * mix;
	const auto dry = volume * (1.0f - mix);

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		if (type == 1)
		{
			PROCESS_COMPRESSOR(m_compressor)
		}
		else if (type == 2)
		{
			PROCESS_COMPRESSOR(m_optoCompressor)
		}
		else if (type == 3)
		{
			PROCESS_COMPRESSOR(m_slewCompressor)
		}
		else if (type == 4)
		{
			PROCESS_COMPRESSOR(m_dualCompressor)
		}
		else
		{
			PROCESS_COMPRESSOR(m_adaptiveCompressor)
		}
	}
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new CompressorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout CompressorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  1.0f,    5.0f, 1.0f,  1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.1f, 200.0f, 0.1f,  0.4f),   10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   5.0f, 600.0f, 0.1f,  0.4f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   1.5f,   8.0f, 0.5f,  1.0f),   4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f, 100.0f, 1.0f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f, 100.0f, 1.0f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f, 100.0f, 1.0f,  1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}
