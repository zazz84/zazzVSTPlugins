/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string SimpleCompressorAudioProcessor::paramsNames[] = { "Threshold", "Attack", "Release", "Volume" };
const std::string SimpleCompressorAudioProcessor::paramsUnitNames[] = { " dB", " ms", " ms", "dB" };

//==============================================================================
SimpleCompressorAudioProcessor::SimpleCompressorAudioProcessor()
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
	thresholdParameter    = apvts.getRawParameterValue(paramsNames[0]);
	attackParameter  = apvts.getRawParameterValue(paramsNames[1]);
	releaseParameter = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter  = apvts.getRawParameterValue(paramsNames[3]);
}

SimpleCompressorAudioProcessor::~SimpleCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_compressor[0].init(sr);
	m_compressor[1].init(sr);
}

void SimpleCompressorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto threshold = thresholdParameter->load();
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();	
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto gain = juce::Decibels::decibelsToGain(-20.0f - threshold);
	const auto outGain = volume / gain;

	buffer.applyGain(gain);

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		// Compressor
		auto& compressor = m_compressor[channel];
		compressor.set(attack, release);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			float& in = channelBuffer[sample];

			// Writte
			in = outGain * compressor.process(in);
		}
	}
}

//==============================================================================
bool SimpleCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleCompressorAudioProcessor::createEditor()
{
    return new SimpleCompressorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SimpleCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SimpleCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleCompressorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(-40.0f, 0.0f, 1.0f, 1.0f), -20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 1.0f, 100.0f, 1.0f, 0.5f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 5.0f, 400.0f, 1.0f, 0.5f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleCompressorAudioProcessor();
}
