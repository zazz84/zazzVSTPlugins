/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string SubBassAudioProcessor::paramsNames[] = { "Sidechain", "Threshold", "Lenght", "Frequency", "Amount", "Volume" };
const std::string SubBassAudioProcessor::paramsUnitNames[] = { " Hz", " dB", "", " Hz", " %", " dB" };
const float SubBassAudioProcessor::FREQUENCY_MIN = 20.0f;
const float SubBassAudioProcessor::FREQUENCY_MAX = 180.0f;

//==============================================================================
SubBassAudioProcessor::SubBassAudioProcessor()
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
	sidechainParameter = apvts.getRawParameterValue(paramsNames[0]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[1]);
	lenghtParameter    = apvts.getRawParameterValue(paramsNames[2]);
	frequencyParameter = apvts.getRawParameterValue(paramsNames[3]);
	amountParameter    = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[5]);
}

SubBassAudioProcessor::~SubBassAudioProcessor()
{
}

//==============================================================================
const juce::String SubBassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SubBassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SubBassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SubBassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SubBassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SubBassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SubBassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SubBassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SubBassAudioProcessor::getProgramName (int index)
{
    return {};
}

void SubBassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SubBassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_delay[0].init(sr, FREQUENCY_MIN);
	m_delay[1].init(sr, FREQUENCY_MIN);

	m_triggerFilter[0].init(sr);
	m_triggerFilter[1].init(sr);

	m_peakDetector[0].init(sr);
	m_peakDetector[1].init(sr);

	constexpr float decayTime = 80.0f;
	m_peakDetector[0].set(decayTime);
	m_peakDetector[1].set(decayTime);
}

void SubBassAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SubBassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SubBassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto sidechain = sidechainParameter->load();
	const auto threshold = juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto lenght = lenghtParameter->load();
	const auto frequency = frequencyParameter->load();
	const auto amount = 0.01f * amountParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	const auto lenghtTime = 2.0f * lenght;
	const auto amountGain = amount * remap(frequency, FREQUENCY_MIN, FREQUENCY_MAX, 24.0f, 4.0f)
								   * remap(frequency, FREQUENCY_MIN, 3.0f * FREQUENCY_MIN, 8.0f, 1.0f)
								   * remap(lenght, 0.0f, 1.0f, 1.0f, 0.25f);

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//Filters references
		auto& delay = m_delay[channel];
		auto& triggerFilter = m_triggerFilter[channel];
		auto& peakDetector = m_peakDetector[channel];
		auto& samplesOpen = m_samplesOpen[channel];

		//Set filters
		triggerFilter.setBandPassPeakGain(sidechain, 0.7f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];
			
			// Trigger
			float trigger = triggerFilter.processDF1(in);

			// Get peak
			const float peak = peakDetector.process(trigger);

			// Generate sub bass
			float subBass = 0.0f;

			if (peak > threshold)
			{
				samplesOpen++;
				
				subBass = in;
								
				const float frequencyNew = remap((float)samplesOpen, 0.0f, sampleRate * 2.0f * lenghtTime, frequency, FREQUENCY_MIN);
				delay.set(frequencyNew, lenghtTime);
			}
			else
			{
				samplesOpen = 0;
			}

			subBass = delay.process(subBass);

			//Out
			channelBuffer[sample] = gain * (in + amountGain * subBass);
		}
	}
}

//==============================================================================
bool SubBassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SubBassAudioProcessor::createEditor()
{
    return new SubBassAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SubBassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SubBassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SubBassAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 1.0f),  80.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -60.0f,   0.0f,  1.0f, 1.0f), -24.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),   0.5f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 1.0f),  80.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SubBassAudioProcessor();
}
