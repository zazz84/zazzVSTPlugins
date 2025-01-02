/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================

const std::string NoiseEnhancerAudioProcessor::paramsNames[] = { "Frequency", "Threshold", "Attack", "Release", "HP", "LP", "Noise", "Volume" };

//==============================================================================
NoiseEnhancerAudioProcessor::NoiseEnhancerAudioProcessor()
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
	thresholdParameter = apvts.getRawParameterValue(paramsNames[1]);
	attackParameter    = apvts.getRawParameterValue(paramsNames[2]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[3]);
	HPParameter        = apvts.getRawParameterValue(paramsNames[4]);
	LPParameter        = apvts.getRawParameterValue(paramsNames[5]);
	noiseParameter     = apvts.getRawParameterValue(paramsNames[6]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[7]);

	buttonSParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonS"));
	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Button1"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Button2"));
	buttonNoiseSParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonNoiseS"));
	buttonDParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonD"));
}

NoiseEnhancerAudioProcessor::~NoiseEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String NoiseEnhancerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoiseEnhancerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoiseEnhancerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoiseEnhancerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoiseEnhancerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoiseEnhancerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoiseEnhancerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoiseEnhancerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoiseEnhancerAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoiseEnhancerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoiseEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	m_envelopeFollowerFilter[0].init((int)(sampleRate));
	m_envelopeFollowerFilter[1].init((int)(sampleRate));

	m_triggerEnvelopeFollower[0].init((int)(sampleRate));
	m_triggerEnvelopeFollower[1].init((int)(sampleRate));

	m_noiseEnvelopeFollower[0].init((int)(sampleRate));
	m_noiseEnvelopeFollower[1].init((int)(sampleRate));

	m_highPassFilter[0].init((int)(sampleRate));
	m_highPassFilter[1].init((int)(sampleRate));

	m_lowPassFilter[0].init((int)(sampleRate));
	m_lowPassFilter[1].init((int)(sampleRate));

	const int channels = getTotalNumOutputChannels();
	if (channels <= 1)
		return;

	// Offset right channel
	for (int i = 0; i < 48000; i++)
	{
		m_whiteNoiseGenerator[1].process();
	}
}

void NoiseEnhancerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoiseEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NoiseEnhancerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const float frequency = frequencyParameter->load();
	const float threshold = thresholdParameter->load();
	const float attack = attackParameter->load();
	const float release = releaseParameter->load();
	const float HPFrequency = HPParameter->load();
	const float LPFrequency = LPParameter->load();
	const float noise = noiseParameter->load();
	const float volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Buttons
	const auto buttonS = buttonSParameter->get();
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto buttonNoiseS = buttonNoiseSParameter->get();
	const auto buttonD = buttonDParameter->get();

	// Mics constants
	const float noiseGain = juce::Decibels::decibelsToGain(24.0f - 60.0f * (1.0f - noise));
	const float thresholdGain = juce::Decibels::decibelsToGain(threshold);
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& envelopeFollowerFilter = m_envelopeFollowerFilter[channel];
		envelopeFollowerFilter.setCoef(frequency, 0.8f);

		auto& triggerEnvelopeFollower = m_triggerEnvelopeFollower[channel];
		triggerEnvelopeFollower.setCoef(0.05f, 20.0f);

		auto& noiseEnvelopeFollower = m_noiseEnvelopeFollower[channel];
		noiseEnvelopeFollower.setCoef(attack, release);

		auto& highPassFilter = m_highPassFilter[channel];
		highPassFilter.setCoef(HPFrequency);

		auto& lowPassFilter = m_lowPassFilter[channel];
		lowPassFilter.setCoef(LPFrequency);

		auto& whiteNoiseGenerator = m_whiteNoiseGenerator[channel];
		float noiseVolumeType = 1.0f;
		if (button1)
		{
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::Normal);
			noiseVolumeType = juce::Decibels::decibelsToGain(-12.0f);
		}
		else
		{
			whiteNoiseGenerator.setDistributionType(WhiteNoiseGenerator::DistributionType::PieceWise);
			noiseVolumeType = juce::Decibels::decibelsToGain(0.0f);
		}

		for (int sample = 0; sample < samples; ++sample)
		{
			// Get input
			const float in = channelBuffer[sample];

			// Input filter
			const float inFilter = envelopeFollowerFilter.process(in);

			// Envelope follower
			const float inEnvelopeFollower = triggerEnvelopeFollower.process(inFilter);

			// Get noise volume
			const float aboveThreshold = fmaxf(0.0f, inEnvelopeFollower - thresholdGain);

			// Noise AR envelope
			const float noiseVolume = noiseEnvelopeFollower.process(aboveThreshold);

			// Generate noise
			const float noise = noiseVolume * noiseVolumeType * whiteNoiseGenerator.process();

			// Hight pass
			const float noiseHP = highPassFilter.process(noise);

			// Dynamic low pass
			if (buttonD)
			{
				const float factor = 0.8f;
				const float noiseVolumeNormalized = fminf(factor, fmaxf(0.0f, noiseVolume)) / factor;
				const float lowPassFactor = LPFrequency * (1.0f - noiseVolumeNormalized);
				lowPassFilter.setCoef(LPFrequency - lowPassFactor);
			}
			const float noiseHPLP = lowPassFilter.process(noiseHP);

			// Apply volume
			if (buttonS)
			{
				channelBuffer[sample] = volume * inFilter;
			}
			else if (buttonNoiseS)
			{
				channelBuffer[sample] = volume * noiseHPLP;
			}
			else
			{
				channelBuffer[sample] = volume * (noiseGain * noiseHPLP + in);
			}
		}
	}
}

//==============================================================================
bool NoiseEnhancerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoiseEnhancerAudioProcessor::createEditor()
{
    return new NoiseEnhancerAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void NoiseEnhancerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void NoiseEnhancerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout NoiseEnhancerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 40.0f, 20000.0f,  1.0f, 0.3f),  1000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(-60.0f,     0.0f,  1.0f, 1.0f),   -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  0.0f,   100.0f,  0.1f, 1.0f),    10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  0.0f,   100.0f,  0.1f, 1.0f),    10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( 100.0f, 10000.0f,  1.0f, 0.3f),   100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(1000.0f, 20000.0f,  1.0f, 0.3f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(  0.0f,     1.0f, 0.01f, 1.0f),     0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(-12.0f,    12.0f,  0.1f, 1.0f),     0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonS", "ButtonS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Button1", "Button1", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("Button2", "Button2", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonNoiseS", "ButtonNoiseS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonD", "ButtonD", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseEnhancerAudioProcessor();
}
