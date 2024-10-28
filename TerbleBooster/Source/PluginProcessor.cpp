/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string TerbleBoosterAudioProcessor::paramsNames[] = { "Frequency", "Mix", "Volume" };

//==============================================================================
TerbleBoosterAudioProcessor::TerbleBoosterAudioProcessor()
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
	frequencyParameter = apvts.getRawParameterValue(paramsNames[0]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[2]);
}

TerbleBoosterAudioProcessor::~TerbleBoosterAudioProcessor()
{
}

//==============================================================================
const juce::String TerbleBoosterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TerbleBoosterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TerbleBoosterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TerbleBoosterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TerbleBoosterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TerbleBoosterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TerbleBoosterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TerbleBoosterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TerbleBoosterAudioProcessor::getProgramName (int index)
{
    return {};
}

void TerbleBoosterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TerbleBoosterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int frequencyFactor = (int)(sampleRate / 48000.0);
	const int samples = 13 * frequencyFactor;
	m_filter[0].init(samples);
	m_filter[1].init(samples);
}

void TerbleBoosterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TerbleBoosterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TerbleBoosterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	const float sampleRate = (float)getSampleRate();
	const float frequencyFactor = sampleRate / 54000.0f;

	// Get params
	const auto frequency = 3.0f - frequencyFactor + 12.0f * frequencyParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto mixInverse = 1.0f - mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto volumeFactor = sampleRate / 44100.0f;
	const auto gain = (3.0f * 10.0f) * frequency * volumeFactor;
	const auto gainMix = mix * gain;
	
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& inLast = m_inLast[channel];
		auto& filter = m_filter[channel];
		filter.set(frequency * frequencyFactor);

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];
			
			const float diff = in - inLast;
			const float out = filter.process(diff);
			inLast = in;

			channelBuffer[sample] = volume * (mixInverse * in + gainMix * out);
		}
	}
}

//==============================================================================
bool TerbleBoosterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TerbleBoosterAudioProcessor::createEditor()
{
    return new TerbleBoosterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void TerbleBoosterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void TerbleBoosterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout TerbleBoosterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f, 1.0f, 0.001f, 1.0f), 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TerbleBoosterAudioProcessor();
}
