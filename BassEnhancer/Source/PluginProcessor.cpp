/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string BassEnhancerAudioProcessor::paramsNames[] = { "Frequency", "Ammount", "Volume" };

//==============================================================================
BassEnhancerAudioProcessor::BassEnhancerAudioProcessor()
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
	ammountParameter       = apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[2]);
}

BassEnhancerAudioProcessor::~BassEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String BassEnhancerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BassEnhancerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BassEnhancerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BassEnhancerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BassEnhancerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BassEnhancerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BassEnhancerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BassEnhancerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BassEnhancerAudioProcessor::getProgramName (int index)
{
    return {};
}

void BassEnhancerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BassEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_resonanceFilter[0].init(sr);
	m_resonanceFilter[1].init(sr);

	m_allPassFilter[0].init(sr);
	m_allPassFilter[1].init(sr);

	m_lowPassFilter[0].init(sr);
	m_lowPassFilter[1].init(sr);

	m_highPassFilter[0].init(sr);
	m_highPassFilter[1].init(sr);
}

void BassEnhancerAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BassEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BassEnhancerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto frequency = frequencyParameter->load();
	const auto ammount = ammountParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	constexpr float maxVolume = 18.0f;
	const auto resonanceGain = juce::Decibels::decibelsToGain(maxVolume);
	const auto lowGain = 0.5f * gain * resonanceGain * juce::Decibels::decibelsToGain(-2.0f * maxVolume * (1.0f - ammount));
	
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//Filters references
		auto& lowPasssFilter = m_lowPassFilter[channel];
		auto& highPasssFilter = m_highPassFilter[channel];
		auto& resonanceFilter = m_resonanceFilter[channel];
		auto& allPassFilter = m_allPassFilter[channel];

		//Set filters
		lowPasssFilter.setLowPass(frequency, 1.0f);
		highPasssFilter.setHighPass(frequency, 2.5f);
		resonanceFilter.setBandPassPeakGain(frequency, 2.0f);
		allPassFilter.setFrequency(0.5f * frequency);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
			
			// Split
			float inLow = 2.0f * lowPasssFilter.processDF1(in);
			float inHigh = highPasssFilter.processDF1(in);
			
			// AllPass hight
			inHigh = allPassFilter.process(inHigh);

			// Distort low
			// Chebyshev polynom
			inLow = 2.0f * inLow * inLow - 1.0f;
			// ARRY waveshaper
			inLow = 1.5f * inLow * (1.0f - inLow * inLow / 3.0f);

			// Filter low
			inLow = resonanceFilter.processDF1(inLow);
			
			// AllPass low
			inLow = allPassFilter.process(inLow);
				
			// Writte
			channelBuffer[sample] = gain * in + lowGain * inLow;
		}
	}
}

//==============================================================================
bool BassEnhancerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BassEnhancerAudioProcessor::createEditor()
{
    return new BassEnhancerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BassEnhancerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BassEnhancerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BassEnhancerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  40.0f, 200.0f,  1.0f, 1.0f), 80.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),  0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassEnhancerAudioProcessor();
}
