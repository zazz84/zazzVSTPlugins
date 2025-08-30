/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string AMTransmitterEmulationAudioProcessor::paramsNames[] = { "Modulation Depth", "Frequency",  "Volume" };
const std::string AMTransmitterEmulationAudioProcessor::paramsUnitNames[] = { " dB", " Hz", " dB" };

//==============================================================================
AMTransmitterEmulationAudioProcessor::AMTransmitterEmulationAudioProcessor()
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
	modulationDepthParameter    = apvts.getRawParameterValue(paramsNames[0]);
	tuneFrequencyParameter		= apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter				= apvts.getRawParameterValue(paramsNames[2]);
}

AMTransmitterEmulationAudioProcessor::~AMTransmitterEmulationAudioProcessor()
{
}

//==============================================================================
const juce::String AMTransmitterEmulationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AMTransmitterEmulationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AMTransmitterEmulationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AMTransmitterEmulationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AMTransmitterEmulationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AMTransmitterEmulationAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AMTransmitterEmulationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AMTransmitterEmulationAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AMTransmitterEmulationAudioProcessor::getProgramName (int index)
{
    return {};
}

void AMTransmitterEmulationAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AMTransmitterEmulationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	
	m_carrierOscillator[0].init(sr);
	m_carrierOscillator[1].init(sr);

	m_outputFilter[0].init(sr);
	m_outputFilter[1].init(sr);
	
	const float frequency = std::fminf(0.5f * (float)(sampleRate), 20000.0f);
	m_outputFilter[0].set(frequency);
	m_outputFilter[1].set(frequency);
}

void AMTransmitterEmulationAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AMTransmitterEmulationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AMTransmitterEmulationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto modulationGain = juce::Decibels::decibelsToGain(modulationDepthParameter->load());
	const auto tuneFrequency = juce::Decibels::decibelsToGain(tuneFrequencyParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	const auto sampleRateHalf = 0.5f * sampleRate;
	const auto carrierFrequency = (0.8f * sampleRateHalf) + tuneFrequency;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// References
		auto& carrierOscillator = m_carrierOscillator[channel];
		auto& outputFilter = m_outputFilter[channel];
		carrierOscillator.set(carrierFrequency);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			float out = channelBuffer[sample];

			// AM modulation
			out = (1.0f + modulationGain * out) * carrierOscillator.process();

			// Output filter
			out = outputFilter.process(out);
		
			//Out
			channelBuffer[sample] = out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool AMTransmitterEmulationAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AMTransmitterEmulationAudioProcessor::createEditor()
{
    return new AMTransmitterEmulationAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void AMTransmitterEmulationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AMTransmitterEmulationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AMTransmitterEmulationAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -10000.0f,  10000.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AMTransmitterEmulationAudioProcessor();
}
