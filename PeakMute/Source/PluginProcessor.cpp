/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string PeakMuteAudioProcessor::paramsNames[] = { "Threshold", "Attenuate", "Recovery", "Volume" };
const std::string PeakMuteAudioProcessor::paramsUnitNames[] = { " dB", " dB", " ms", " dB" };

//==============================================================================
PeakMuteAudioProcessor::PeakMuteAudioProcessor()
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
	thresholdParameter = apvts.getRawParameterValue(paramsNames[0]);
	attenuateParameter = apvts.getRawParameterValue(paramsNames[1]);
	recoveryParameter  = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);
}

PeakMuteAudioProcessor::~PeakMuteAudioProcessor()
{
}

//==============================================================================
const juce::String PeakMuteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PeakMuteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PeakMuteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PeakMuteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PeakMuteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PeakMuteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PeakMuteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PeakMuteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PeakMuteAudioProcessor::getProgramName (int index)
{
    return {};
}

void PeakMuteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PeakMuteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_peakDetector[0].init(sr);
	m_peakDetector[1].init(sr);
}

void PeakMuteAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PeakMuteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PeakMuteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	if (!isNonRealtime())
	{
		// Get params
		const auto threshold = juce::Decibels::decibelsToGain(thresholdParameter->load());
		const auto attenuate = (attenuateParameter->load() == -60.0) ? 0.0f : juce::Decibels::decibelsToGain(attenuateParameter->load());
		const auto recovery = recoveryParameter->load();
		const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

		// Mics constants
		const auto channels = getTotalNumOutputChannels();
		const auto samples = buffer.getNumSamples();

		for (int channel = 0; channel < channels; ++channel)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);

			//Filters references
			auto& peakDetector = m_peakDetector[channel];
			peakDetector.set(recovery);

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				float inPeakDetector = 0.0f;
				if (in > threshold || in < -threshold)
				{
					inPeakDetector = 1.0f;
				}

				const float outPeakDetector = peakDetector.process(inPeakDetector);

				//Out
				channelBuffer[sample] = in * (attenuate + (1.0f - attenuate) * (1.0f - outPeakDetector));
			}
		}

		buffer.applyGain(gain);
	}
}

//==============================================================================
bool PeakMuteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PeakMuteAudioProcessor::createEditor()
{
    return new PeakMuteAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void PeakMuteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void PeakMuteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout PeakMuteAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -24.0f, 24.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -60.0f, 0.0f, 1.0f, 1.0f), -60.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  10.0f, 1000.0f, 1.0f, 0.6f), 300.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PeakMuteAudioProcessor();
}
