/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string MSAudioProcessor::paramsNames[] = { "MGain", "SGain", "MPan", "SPan", "Volume" };

//==============================================================================
MSAudioProcessor::MSAudioProcessor()
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
	mGainParameter = apvts.getRawParameterValue(paramsNames[0]);
	sGainParameter = apvts.getRawParameterValue(paramsNames[1]);
	mPanParameter = apvts.getRawParameterValue(paramsNames[2]);
	sPanParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

MSAudioProcessor::~MSAudioProcessor()
{
}

//==============================================================================
const juce::String MSAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MSAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MSAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MSAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MSAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MSAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MSAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MSAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MSAudioProcessor::getProgramName (int index)
{
    return {};
}

void MSAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MSAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void MSAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MSAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void MSAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	if (getTotalNumOutputChannels() != 2)
	{
		return;
	}

	// Get params
	const auto mGain = juce::Decibels::decibelsToGain(mGainParameter->load());
	const auto sGain = juce::Decibels::decibelsToGain(sGainParameter->load());
	const auto mPan = mPanParameter->load();
	const auto sPan = sPanParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto samples = buffer.getNumSamples();

	const float mPanL = volume * (0.5f - 0.5f * mPan);
	const float mPanR = volume * (0.5f + 0.5 * mPan);
	const float sPanL = volume * (0.5f - 0.5f * sPan);
	const float sPanR = volume * (0.5f + 0.5 * sPan);

	auto* LChannel = buffer.getWritePointer(0);
	auto* RChannel = buffer.getWritePointer(1);

	for (int sample = 0; sample < samples; sample++)
	{
		const float LIn = LChannel[sample];
		const float RIn = RChannel[sample];

		const float mid = mGain * (LIn + RIn);
		const float side = sGain * (LIn - RIn);
			
		LChannel[sample] = mPanL * mid + sPanL * side;
		RChannel[sample] = mPanR * mid - sPanR * side;
	}
}

//==============================================================================
bool MSAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MSAudioProcessor::createEditor()
{
    return new MSAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MSAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MSAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MSAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MSAudioProcessor();
}
