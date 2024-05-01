/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string RoomReverbAudioProcessor::paramsNames[] = { "Time", "Resonance", "Damping", "Width", "CSeed", "ASeed", "TimeMin", "Complexity", "Mix", "Volume" };
const float RoomReverbAudioProcessor::m_dampingFrequencyMin = 220.0f;

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
	timeParameter            = apvts.getRawParameterValue(paramsNames[0]);
	resonanceParameter       = apvts.getRawParameterValue(paramsNames[1]);
	dampingParameter         = apvts.getRawParameterValue(paramsNames[2]);
	widthParameter           = apvts.getRawParameterValue(paramsNames[3]);
	combFilterSeedParameter  = apvts.getRawParameterValue(paramsNames[4]);
	allPassSeedParameter     = apvts.getRawParameterValue(paramsNames[5]);
	timeMinParameter         = apvts.getRawParameterValue(paramsNames[6]);
	complexityParameter      = apvts.getRawParameterValue(paramsNames[7]);
	mixParameter             = apvts.getRawParameterValue(paramsNames[8]);
	volumeParameter          = apvts.getRawParameterValue(paramsNames[9]);
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
	int delayCombFilterSamples[MAX_COMPLEXITY];
	int delayAllPassSamples[MAX_COMPLEXITY];

	for (int i = 0; i < MAX_COMPLEXITY; i++)
	{
		delayCombFilterSamples[i] = (int)(4.0f * COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate);
		delayAllPassSamples[i] = (int)(4.0f * ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate);
	}

	m_circularCombFilter[0].init(MAX_COMPLEXITY, delayCombFilterSamples, delayAllPassSamples);
	m_circularCombFilter[1].init(MAX_COMPLEXITY, delayCombFilterSamples, delayAllPassSamples);
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
	const auto combFilterResonance = 0.4f - (0.2f * (1.0f - combFilterTime)) + 0.4f * resonanceParameter->load();
	const auto allPassTime = combFilterTime;
	const auto allPassResonance = 0.7f * combFilterResonance;
	const auto width = widthParameter->load();;
	const auto damping = dampingParameter->load();
	const long combFilterSeed =  (long)(combFilterSeedParameter->load());
	const long allPassSeed = (long)(allPassSeedParameter->load());
	const auto timeMin = timeMinParameter->load();
	const int complexity = (int)(complexityParameter->load());
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
		
		// Generate circular comb filter parameters
		int combFilterDelaySamples[MAX_COMPLEXITY];
		int allPassDelaySamples[MAX_COMPLEXITY];
		float combFilterFeedback[MAX_COMPLEXITY];
		float allPassFeedback[MAX_COMPLEXITY];
		float dampingFrequency[MAX_COMPLEXITY];

		const auto combFilterWidth = (channel == 0) ? 1.0f - width * 0.1f : 1.0f;
		const auto allPassWidth = (channel == 1) ? 1.0f - width * 0.1f : 1.0f;

		m_noiseGenerator.setSeed(allPassSeed);

		for (int i = 0; i < MAX_COMPLEXITY; i++)
		{		
			const float sing = (i % 2 == 0) ? -1.0f : 1.0f;
			combFilterFeedback[i] = sing * combFilterResonance;
			allPassFeedback[i] = allPassResonance * 0.9f + 0.2 * m_noiseGenerator.process();
			//dampingFrequency[i] = fminf(frequencyMax, 3000.0f + (MAX_COMPLEXITY - i) * 500.0f + (1.0f - damping) * 18000.0f);
			dampingFrequency[i] = fminf(frequencyMax, m_dampingFrequencyMin + (1.0f - damping) * (frequencyMax - m_dampingFrequencyMin) * 0.9f + 0.2 * m_noiseGenerator.process());
		}

		m_noiseGenerator.setSeed(combFilterSeed);
		const float combFilterTimeFactor = COMB_FILTER_MAX_TIME_MS * 0.001f * sampleRate;
		int combFilterSum = 0;

		for (int i = 0; i < MAX_COMPLEXITY; i++)
		{
			const float rnd = timeMin + m_noiseGenerator.process() * (1.0f - timeMin);
			const int samples = (int)(combFilterWidth * combFilterTime * rnd * combFilterTimeFactor);
			combFilterDelaySamples[i] = samples;
			combFilterSum += samples;
		}

		m_noiseGenerator.setSeed(allPassSeed);
		const float allPassTimeFactor = ALL_PASS_MAX_TIME_MS * 0.001f * sampleRate;
		int allPassSum = 0;

		for (int i = 0; i < MAX_COMPLEXITY; i++)
		{
			const float rnd = timeMin + m_noiseGenerator.process() * (1.0f - timeMin);
			const int samples = (int)(allPassWidth * allPassTime * rnd * allPassTimeFactor);
			allPassDelaySamples[i] = samples;
			allPassSum += samples;
		}

		//Normalize
		const float combFilterNormalizeFactor = combFilterSum / (MAX_COMPLEXITY * combFilterTime * combFilterTimeFactor);
		const float allPassNormalizeFactor = allPassSum / (MAX_COMPLEXITY * allPassTime * allPassTimeFactor);

		for (int i = 0; i < MAX_COMPLEXITY; i++)
		{
			combFilterDelaySamples[i] = combFilterDelaySamples[i] * combFilterNormalizeFactor;
			allPassDelaySamples[i] = allPassDelaySamples[i] * allPassNormalizeFactor;
		}

		// Set circular comb filter
		circularCombFilter.setComplexity(complexity);
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
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f, (float)0x7fff, 1.00f), (float)(0x7fff * 0.25f)));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f, (float)0x7fff, 1.00f), (float)(0x7fff * 0.5f)));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   1.0f, (float)MAX_COMPLEXITY, 1.00f, 1.0f), 8.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>( -12.0f, 12.0f, 0.10f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomReverbAudioProcessor();
}
