/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string NEDDelayAudioProcessor::paramsNames[] = { "Time", "Feedback", "Low Cut", "High Cut", "Exponential", "Mix", "Volume" };
const std::string NEDDelayAudioProcessor::paramsUnitNames[] = { " ms", "", " Hz", " Hz", " %", " %", " dB" };

//==============================================================================
NEDDelayAudioProcessor::NEDDelayAudioProcessor()
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
	timeParameter = apvts.getRawParameterValue(paramsNames[0]);
	feedbackParameter = apvts.getRawParameterValue(paramsNames[1]);
	lowCutFrequencyParameter = apvts.getRawParameterValue(paramsNames[2]);
	highCutFrequencyParameter = apvts.getRawParameterValue(paramsNames[3]);
	exponentialParameter = apvts.getRawParameterValue(paramsNames[4]);
	mixParameter = apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[6]);
}

NEDDelayAudioProcessor::~NEDDelayAudioProcessor()
{
}

//==============================================================================
const juce::String NEDDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NEDDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NEDDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NEDDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NEDDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NEDDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NEDDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NEDDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NEDDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void NEDDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NEDDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int size = int((float)TIME_MAX * 0.001f * (float)sampleRate);
	
	m_combFilter[0].init(size);
	m_combFilter[1].init(size);

	const int sr = (int)sampleRate;

	m_lowCutFilter[0].init(sr);
	m_lowCutFilter[1].init(sr);
	m_highCutFilter[0].init(sr);
	m_highCutFilter[1].init(sr);

	m_delay[0].init(sr, size);
	m_delay[1].init(sr, size);
}

void NEDDelayAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NEDDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo());

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NEDDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto time = timeParameter->load();
	const auto feedback = (int)(std::round(feedbackParameter->load()));
	const auto lowCutFrequency = lowCutFrequencyParameter->load();
	const auto highCutFrequency = highCutFrequencyParameter->load();
	const auto exponentialWet = 0.01f * exponentialParameter->load();
	const auto wet = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto samples = buffer.getNumSamples();
	const auto channels = buffer.getNumChannels();
	const auto dry = 1.0f - wet;
	const auto exponentialDry = 1.0f - exponentialWet;
	const auto exponentialFeedback = 1.0f - 1.0f / feedback;
	const int size = (int)(0.001 * (double)time * getSampleRate());


	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		auto& combFilter = m_combFilter[channel];
		auto& lowCutFilter = m_lowCutFilter[channel];
		auto& highCutFilter = m_highCutFilter[channel];
		auto& delay = m_delay[channel];
		
		lowCutFilter.setHighPass(lowCutFrequency, 0.7f);
		highCutFilter.setLowPass(highCutFrequency, 0.7f);

		combFilter.set(feedback, size);
		delay.set(exponentialFeedback, size);

		for (int sample = 0; sample < samples; sample++)
		{			
			const float in = channelBuffer[sample];

			const float ned = exponentialDry * combFilter.process(in);
			const float exponential = exponentialWet * delay.process(in);
	
			float out = lowCutFilter.processDF1(highCutFilter.processDF1(ned + exponential));

			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(volume);
}

//==============================================================================
bool NEDDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NEDDelayAudioProcessor::createEditor()
{
    return new NEDDelayAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void NEDDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void NEDDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout NEDDelayAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,500.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   1.0f,  20.0f,  1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   20.0f,  20000.0f,  1.0f, 0.4f), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   20.0f,  20000.0f,  1.0f, 0.4f), 16000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NEDDelayAudioProcessor();
}
