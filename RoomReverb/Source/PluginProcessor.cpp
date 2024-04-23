/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string RoomReverbAudioProcessor::paramsNames[] = { "Time", "Resonance", "Damping", "Mix", "Volume" };

const float RoomReverbAudioProcessor::combFilterDelayTimeSeconds[] = { 0.051f, 0.015f, 0.022f, 0.032f, 0.043f, 0.007f, 0.057f, 0.061f };
const float RoomReverbAudioProcessor::allPassDelayTimeSeconds[] = { 0.011f, 0.032f, 0.082f, 0.045f, 0.019f, 0.092f, 0.043f, 0.048f };
const float RoomReverbAudioProcessor::combFilterFeedbackFactor[] = { 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f };
const float RoomReverbAudioProcessor::allPassFeedbackFactor[] = { 1.1f, 0.95f, 1.05f, 1.0f, 1.15f, 0.55f, 0.97f, 1.03f };

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
	timeParameter      = apvts.getRawParameterValue(paramsNames[0]);
	resonanceParameter = apvts.getRawParameterValue(paramsNames[1]);
	dampingParameter             = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter                 = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter              = apvts.getRawParameterValue(paramsNames[4]);
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
	int delayCombFilterSamples[N_COMPLEXITY];
	int delayAllPassSamples[N_COMPLEXITY];

	for (int i = 0; i < N_COMPLEXITY; i++)
	{
		delayCombFilterSamples[i] = (int)(combFilterDelayTimeSeconds[i] * sampleRate);
		delayAllPassSamples[i] = (int)(allPassDelayTimeSeconds[i] * sampleRate);
	}

	m_circularCombFilter[0].init(N_COMPLEXITY, delayCombFilterSamples, delayAllPassSamples);
	m_circularCombFilter[1].init(N_COMPLEXITY, delayCombFilterSamples, delayAllPassSamples);
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
	// Get params
	const auto combFilterTime = timeParameter->load();
	const auto combFilterResonance = 0.7f * (0.7f + 0.3f * resonanceParameter->load());
	const auto allPassTime = 0.85f * combFilterTime;
	const auto allPassResonance = 0.57f * combFilterResonance;
	const auto damping = dampingParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();
	const auto sampleRate = getSampleRate();
	const float frequencyMax = sampleRate * 0.45f;

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// Circular comb filter pointer
		auto& circularCombFilter = m_circularCombFilter[channel];
		
		int combFilterDelaySamples[N_COMPLEXITY];
		int allPassDelaySamples[N_COMPLEXITY];
		float combFilterFeedback[N_COMPLEXITY];
		float allPassFeedback[N_COMPLEXITY];
		float dampingFrequency[N_COMPLEXITY];

		for (int i = 0; i < N_COMPLEXITY; i++)
		{
			combFilterDelaySamples[i] = (int)(combFilterTime * combFilterDelayTimeSeconds[i] * sampleRate);
			allPassDelaySamples[i] = (int)(allPassTime * allPassDelayTimeSeconds[i] * sampleRate);
			combFilterFeedback[i] = combFilterResonance * combFilterFeedbackFactor[i];
			allPassFeedback[i] = allPassResonance * allPassFeedbackFactor[i];
			dampingFrequency[i] = fminf(frequencyMax, 3000.0f + i * 500.0f + (1.0f - damping) * 18000.0f);
		}

		circularCombFilter.setSize(combFilterDelaySamples);
		circularCombFilter.setFeedback(combFilterFeedback);
		circularCombFilter.setAllPassSize(allPassDelaySamples);
		circularCombFilter.setAllPassFeedback(allPassFeedback);

		circularCombFilter.setDampingFrequency(dampingFrequency);

		//=======================================================================

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];
			const float out = 0.3f * circularCombFilter.process(in);

			channelBuffer[sample] = volume * ((1.0f - mix) * in + mix * out);
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

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -12.0f, 12.0f, 0.10f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomReverbAudioProcessor();
}
