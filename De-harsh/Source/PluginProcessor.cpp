/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

//==============================================================================

const std::string DeharshAudioProcessor::paramsNames[] = { "Damping", "Presence", "Saturation", "Mix", "Volume" };
const std::string DeharshAudioProcessor::paramsUnitNames[] = { " dB", " dB", " %", " %", " dB" };

//==============================================================================
DeharshAudioProcessor::DeharshAudioProcessor()
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
	dampingParameter = apvts.getRawParameterValue(paramsNames[0]);
	presenceParameter = apvts.getRawParameterValue(paramsNames[1]);
	saturationParameter = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

DeharshAudioProcessor::~DeharshAudioProcessor()
{
}

//==============================================================================
const juce::String DeharshAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DeharshAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DeharshAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DeharshAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DeharshAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DeharshAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DeharshAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DeharshAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DeharshAudioProcessor::getProgramName (int index)
{
    return {};
}

void DeharshAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DeharshAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_dampingFilter[channel].init(sr);
		m_presenceHighShelfFilter[channel].init(sr);
		m_presencePeakFilter[channel].init(sr);

		m_preEmphasisFilter[channel].init(sr);
		m_deEmphasisFilter[channel].init(sr);
	}
}

void DeharshAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DeharshAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DeharshAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto damping = -dampingParameter->load();
	const auto presence = presenceParameter->load();
	const auto wetSaturation = 0.01f * saturationParameter->load();
	const auto wet = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto dry = 1.0f - wet;
	const auto dampingFilterQ = 0.07f * damping + 3.0f;
	const auto drySaturation = 1.0f - wetSaturation;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& dampingFilter = m_dampingFilter[channel];
		auto& presenceHighShelfFilter = m_presenceHighShelfFilter[channel];
		auto& presencePeakFilter = m_presencePeakFilter[channel];
		
		auto& preEmphasisFilter = m_preEmphasisFilter[channel];
		auto& deEmphasisFilter = m_deEmphasisFilter[channel];

		dampingFilter.setPeak(3136.0f, dampingFilterQ, damping);
		presenceHighShelfFilter.setHighShelf(4186.0f, 1.0f, presence);
		presencePeakFilter.setPeak(6272.0f, 1.0f, 0.33f * presence);

		preEmphasisFilter.setLowShelf(440.0f, 0.707f, -9.0f);
		deEmphasisFilter.setLowShelf(440.0f, 0.707f, 9.0f);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			const float in = channelBuffer[sample];

			float out = in;

			// Filter
			out = dampingFilter.processDF1(out);
			out = presenceHighShelfFilter.processDF1(out);
			out = presencePeakFilter.processDF1(out);

			// Saturation
			float outSat = preEmphasisFilter.processDF1(out);
			outSat = 0.5f * Waveshapers::Reciprocal(outSat, 2.0f);
			outSat = deEmphasisFilter.processDF1(outSat);

			out = drySaturation * out + wetSaturation * outSat;
		
			//Out
			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool DeharshAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DeharshAudioProcessor::createEditor()
{
    return new DeharshAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void DeharshAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DeharshAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout DeharshAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 0.0f, 20.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 0.0f, 6.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f, 18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DeharshAudioProcessor();
}
