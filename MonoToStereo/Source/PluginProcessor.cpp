/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================

const std::string MonoToStereoAudioProcessor::paramsNames[] = { "Delay", "Width", "Color", "Modulation", "Volume" };
const std::string MonoToStereoAudioProcessor::labelNames[] =  { "Delay", "Width", "Color", "Modulation", "Volume" };
const std::string MonoToStereoAudioProcessor::paramsUnitNames[] = { " ms", " %", " %", " %", " dB" };
const float MonoToStereoAudioProcessor::MINIMUM_DELAY_TIME_MS = 1.0f;
const float MonoToStereoAudioProcessor::MAXIMUM_DELAY_TIME_MS = 30.0f;
const float MonoToStereoAudioProcessor::MINIMUM_MODULATION_FREQUENCY = 0.1f;
const float MonoToStereoAudioProcessor::MAXIMUM_MODULATION_FREQUENCY = 1.0f;
const float MonoToStereoAudioProcessor::MAXIMUM_WET = 0.4f;

//==============================================================================
MonoToStereoAudioProcessor::MonoToStereoAudioProcessor()
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
	delayParameter		= apvts.getRawParameterValue(paramsNames[0]);
	widthParameter		= apvts.getRawParameterValue(paramsNames[1]);
	colorParameter		= apvts.getRawParameterValue(paramsNames[2]);
	modulationParameter	= apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter		= apvts.getRawParameterValue(paramsNames[4]);
}

MonoToStereoAudioProcessor::~MonoToStereoAudioProcessor()
{
}

//==============================================================================
const juce::String MonoToStereoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MonoToStereoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MonoToStereoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MonoToStereoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MonoToStereoAudioProcessor::getTailLengthSeconds() const
{
    return 0.001 * static_cast<double>(MAXIMUM_DELAY_TIME_MS);
}

int MonoToStereoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MonoToStereoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MonoToStereoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MonoToStereoAudioProcessor::getProgramName (int index)
{
    return {};
}

void MonoToStereoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MonoToStereoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	m_delayTimeSmoother.init(sr);
	m_delayTimeSmoother.set(2.0f);

	m_modulationSmoother.init(sr);
	m_modulationSmoother.set(2.0f);

	const int samples = static_cast<int>(static_cast<double>(MAXIMUM_DELAY_TIME_MS) * 0.001 * sampleRate);
	m_buffer.init(samples);

	m_colorFilter.init(sr);

	m_oscillator.init(sr);
}

void MonoToStereoAudioProcessor::releaseResources()
{
	m_delayTimeSmoother.release();
	m_modulationSmoother.release();
	m_buffer.release();
	m_oscillator.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MonoToStereoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MonoToStereoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Process only stereo input
	if (buffer.getNumChannels() != 2)
	{
		return;
	}
	
	// Get params
	const auto delayTime = delayParameter->load();
	const auto mix = MAXIMUM_WET * sqrtf(0.01f * widthParameter->load());
	const auto color = 0.01f * colorParameter->load();
	const auto modulation = 0.01f * modulationParameter->load();
	const auto gainCompensation = mix * 12.0f * color;
	const auto gain = Math::dBToGain(volumeParameter->load() + gainCompensation);

	// Mics constants
	const auto sampleRateMS = static_cast<float>(0.001 * getSampleRate());
	const auto samples = buffer.getNumSamples();

	// Left and right channel read pointers
	auto* leftChannel = buffer.getWritePointer(0);
	auto* rightChannel = buffer.getWritePointer(1);

	// Set color filter
	m_colorFilter.setLowShelf(800.0f, 0.707f, -12.0f * color);

	for (int sample = 0; sample < samples; ++sample)
	{
		// Get delayed sample
		const auto delayTimeSmooth = m_delayTimeSmoother.process(delayTime);
		const auto delaySamples = static_cast<int>(delayTimeSmooth * sampleRateMS);
		const auto delayedSample = m_colorFilter.processDF1(m_buffer.readDelay(delaySamples));

		// Handle modulation
		const auto modulationSmooth = m_modulationSmoother.process(modulation);
		const auto modulationFrequency = Math::remap(modulationSmooth, 0.0f, 1.0f, MINIMUM_MODULATION_FREQUENCY, MAXIMUM_MODULATION_FREQUENCY);
		m_oscillator.set(modulationFrequency);
		const auto oscilator = m_oscillator.process();
		const auto wetModulation = (0.5f * 0.5f * MAXIMUM_WET) * oscilator * modulationSmooth;		// Maximum modulation is 50% of width range

		// Get wet modulater
		const auto wetModulated = Math::clamp(mix + wetModulation, 0.0f, MAXIMUM_WET);
		
		// Store current sample
		const float inLeft = leftChannel[sample];
		const float inRight = rightChannel[sample];	
		m_buffer.write(inLeft + inRight);

		// Calculate output
		const float out = gain * wetModulated * delayedSample;
		const float dry = gain * (1.0f - wetModulated);

		// Apply wer/dry
		leftChannel[sample] = dry * inLeft + out;
		rightChannel[sample] = dry * inRight - out;
	}
}

//==============================================================================
bool MonoToStereoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MonoToStereoAudioProcessor::createEditor()
{
    return new MonoToStereoAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MonoToStereoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MonoToStereoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MonoToStereoAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( MINIMUM_DELAY_TIME_MS, MAXIMUM_DELAY_TIME_MS, 0.1f, 0.7f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonoToStereoAudioProcessor();
}
