/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <vector>;

//==============================================================================

const std::string RoomReverbAudioProcessor::paramsNames[] = { "Frequency", "Style", "Mix", "Volume" };

//==============================================================================
RoomReverbAudioProcessor::RoomReverbAudioProcessor()
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
	styleParameter     = apvts.getRawParameterValue(paramsNames[1]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Button1"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Button2"));
}

RoomReverbAudioProcessor::~RoomReverbAudioProcessor()
{
}

//==============================================================================
const juce::String RoomReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RoomReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RoomReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RoomReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RoomReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RoomReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RoomReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RoomReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RoomReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void RoomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RoomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	m_Convolution[0].init(IMPULSE_RESPONSE_SIZE);
	m_Convolution[1].init(IMPULSE_RESPONSE_SIZE);

	m_Convolution[0].setImpulseResponseLenght(IMPULSE_RESPONSE_SIZE);
	m_Convolution[1].setImpulseResponseLenght(IMPULSE_RESPONSE_SIZE);

	std::vector<float> impulseResponse = { 0.8f, 1.0f, -1.0f, 0.5f, -0.5f, 0.25f, -0.25f, 0.3f };

	m_Convolution[0].setImpulseResponse(impulseResponse);
	m_Convolution[1].setImpulseResponse(impulseResponse);
}

void RoomReverbAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RoomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RoomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();

	// Get params
	const auto frequency = frequencyParameter->load();
	const auto style = (button1) ? (frequency - (frequency - FREQUENCY_MIN) * styleParameter->load()) : (0.01f + styleParameter->load() * 2.0f);
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& convolution = m_Convolution[channel];

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];

			const float inConvoluted = convolution.process(in);

			channelBuffer[sample] = volume * ((1.0f - mix) * in + mix * inConvoluted);
		}
	}
}

//==============================================================================
bool RoomReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RoomReverbAudioProcessor::createEditor()
{
    return new RoomReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void RoomReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void RoomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout RoomReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 0.3f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(          0.0f,          1.0f, 0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(          0.0f,          1.0,  0.01f, 1.0f),   0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(        -12.0f,         12.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("Button1", "Button1", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("Button2", "Button2", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomReverbAudioProcessor();
}
