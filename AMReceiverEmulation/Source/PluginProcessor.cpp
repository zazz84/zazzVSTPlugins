/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string AMReceiverEmulationAudioProcessor::paramsNames[] = { "Tune", "Recovery", "Volume" };
const std::string AMReceiverEmulationAudioProcessor::paramsUnitNames[] = { " Hz", " ms", " dB" };

//==============================================================================
AMReceiverEmulationAudioProcessor::AMReceiverEmulationAudioProcessor()
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
	tuningFrequencyParameter	= apvts.getRawParameterValue(paramsNames[0]);
	recoveryParameter			= apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter				= apvts.getRawParameterValue(paramsNames[2]);
}

AMReceiverEmulationAudioProcessor::~AMReceiverEmulationAudioProcessor()
{
}

//==============================================================================
const juce::String AMReceiverEmulationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AMReceiverEmulationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AMReceiverEmulationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AMReceiverEmulationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AMReceiverEmulationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AMReceiverEmulationAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AMReceiverEmulationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AMReceiverEmulationAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AMReceiverEmulationAudioProcessor::getProgramName (int index)
{
    return {};
}

void AMReceiverEmulationAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AMReceiverEmulationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_tuningFilter[0].init(sr);
	m_tuningFilter[1].init(sr);

	m_diodeDetector[0].init(sr);
	m_diodeDetector[1].init(sr);

	m_dcFilter[0].init(sr);
	m_dcFilter[1].init(sr);

	m_dcFilter[0].set(100.0f);
	m_dcFilter[1].set(100.0f);
}

void AMReceiverEmulationAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AMReceiverEmulationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AMReceiverEmulationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Mics constants
	const auto sampleRate = (float)getSampleRate();
	const auto sampleRateHalf = 0.5f * sampleRate;
	
	// Get params
	const auto tuningFrequency = std::fminf(sampleRateHalf, 0.8f * sampleRateHalf + tuningFrequencyParameter->load());
	const auto recovery = recoveryParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// References
		auto& tuningFilter = m_tuningFilter[channel];
		auto& diodeDetector = m_diodeDetector[channel];
		auto& dcFilter = m_dcFilter[channel];
		tuningFilter.setBandPassSkirtGain(tuningFrequency, 20.0f);
		diodeDetector.set(0.0f, recovery);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			float out = channelBuffer[sample];

			// Filter
			out = tuningFilter.processDF1(out);

			// Detect
			out = diodeDetector.process(out);

			// DC filter
			out = dcFilter.process(out);
		
			//Out
			channelBuffer[sample] = out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool AMReceiverEmulationAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AMReceiverEmulationAudioProcessor::createEditor()
{
    return new AMReceiverEmulationAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void AMReceiverEmulationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AMReceiverEmulationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AMReceiverEmulationAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(-20000.0f, 20000.0f, 100.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(0.00001f, 10.0f, 0.00001f, 0.2f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AMReceiverEmulationAudioProcessor();
}
