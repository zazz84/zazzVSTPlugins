/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================

const std::string MonoToStereoAudioProcessor::paramsNames[] =		{ "Delay", "Width", "LS/HS", "HP",  "DynamicAmount", "DynamicSpeed", "Depth", "ModulationSpeed", "Volume" };
const std::string MonoToStereoAudioProcessor::labelNames[] =		{ "Delay", "Width", "LS/HS", "HP",  "Amount",		 "Speed",        "Depth", "Speed",           "Volume" };
const std::string MonoToStereoAudioProcessor::paramsUnitNames[] =	{ " ms",   " %",    " %",    " Hz", " %",            " %",           " %",    " Hz",             " dB" };
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
	delayParameter				= apvts.getRawParameterValue(paramsNames[ParamsName::Delay]);
	widthParameter				= apvts.getRawParameterValue(paramsNames[ParamsName::Width]);
	colorParameter				= apvts.getRawParameterValue(paramsNames[ParamsName::Color]);
	HPParameter					= apvts.getRawParameterValue(paramsNames[ParamsName::HP]);
	dynamicParameter			= apvts.getRawParameterValue(paramsNames[ParamsName::Dynamic]);
	dynamicSpeedParameter		= apvts.getRawParameterValue(paramsNames[ParamsName::DynamicSpeed]);
	modulationDepthParameter	= apvts.getRawParameterValue(paramsNames[ParamsName::ModulationDepth]);
	modulationSpeedParameter	= apvts.getRawParameterValue(paramsNames[ParamsName::ModulationSpeed]);
	volumeParameter				= apvts.getRawParameterValue(paramsNames[ParamsName::Volume]);
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
	m_wetSmoother.init(sr);
	m_wetSmoother.set(20.0f);

	const int samples = static_cast<int>(static_cast<double>(MAXIMUM_DELAY_TIME_MS) * 0.001 * sampleRate);
	m_buffer.init(samples);

	m_colorFilter.init(sr);
	m_HPFilter.init(sr);

	m_oscillator.init(sr);

	m_envelopeSlow.init(sr);
	m_envelopeSlow.set(50, 200.0f);

	m_envelopeFast.init(sr);
	m_envelopeFast.set(1.0f, 40.0f);

	m_envelopeMid.init(sr);
	m_envelopeMid.set(0.3f, 40.0f);
	m_envelopeSide.init(sr);
	m_envelopeSide.set(0.3f, 40.0f);
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
	const auto width = MAXIMUM_WET * sqrtf(0.01f * widthParameter->load());
	const auto color = 0.01f * colorParameter->load();
	const auto hpFrequency = HPParameter->load();
	const auto dynamic = 0.01f * dynamicParameter->load();
	const auto dynamicSpeed = 0.01f * dynamicSpeedParameter->load();
	const auto modulationDepth = 0.01f * modulationDepthParameter->load();
	const auto modulationSpeed = modulationSpeedParameter->load();
	const auto gainCompensation = width * 12.0f * std::fabsf(color);
	const auto gain = Math::dBToGain(volumeParameter->load() + gainCompensation);

	// Mics constants
	const auto sampleRateMS = static_cast<float>(0.001 * getSampleRate());
	const auto samples = buffer.getNumSamples();

	// Set dynamic speed
	m_envelopeSlow.set(50.0f + (1.0f - dynamicSpeed) * 950.0f, 200.0f);

	// Left and right channel read pointers
	auto* leftChannel = buffer.getWritePointer(0);
	auto* rightChannel = buffer.getWritePointer(1);

	// Set color filter
	if (color > 0.0f)
	{
		m_colorFilter.setLowShelf(800.0f, 0.707f, -12.0f * color);
	}
	else
	{
		m_colorFilter.setHighShelf(2000.0f, 0.707f, 12.0f * color);
	}

	m_HPFilter.setHighPass(hpFrequency, 0.707f);

	for (int sample = 0; sample < samples; sample++)
	{
		// Get inputs
		const float inLeft = leftChannel[sample];
		const float inRight = rightChannel[sample];
		const float inSum = inLeft + inRight;
		
		// Get delayed sample
		const auto delayTimeSmooth = m_delayTimeSmoother.process(delayTime);
		const auto delaySamples = static_cast<int>(delayTimeSmooth * sampleRateMS);
		const auto delayedSample = m_HPFilter.processDF1(m_colorFilter.processDF1(m_buffer.readDelay(delaySamples)));

		// Handle dynamic width
		const float envelopeIn = std::fabsf(inSum);
		// Add 1e-6 to avoid division by 0. Envelope output is alway > 0
		const float envelopeSlow = 1e-6f + m_envelopeSlow.process(envelopeIn);
		const float envelopeFast = m_envelopeFast.process(envelopeIn);

		const float difference = envelopeFast / envelopeSlow;
		const float differencedB = Math::gainTodB(difference);

		constexpr float LIMIT_DB = 12.0f;

		float wD = 0.0f;		
		if (dynamic > 0.0f)
		{
			wD = Math::remap(differencedB, 0.0f, LIMIT_DB, 0.0f, 1.0f);
		}
		else
		{
			wD = Math::remap(differencedB, 0.0f, LIMIT_DB, 1.0f, 0.0f);
		}

		// MAYBE???
		// TODO: Better remaping of wD to width
		wD = std::sqrtf(wD);

		const float dynamicAbs = std::fabsf(dynamic);
		const float widthDynamic = dynamicAbs * wD * width + (1.0f - dynamicAbs) * width;
		
		// Handle modulationDepth
		const auto modulationDepthSmooth = m_modulationSmoother.process(modulationDepth);
		m_oscillator.set(modulationSpeed);
		const auto oscilator = m_oscillator.process();
		const auto wetModulated = modulationSpeed > 0.0f ? widthDynamic * (1.0f - 0.5f * (oscilator + 1.0f) * modulationDepthSmooth) : widthDynamic;
		// Smooth because there were some artefacts when dynamic mode (-100%)
		const auto wetModulatedSmooth = m_wetSmoother.process(wetModulated);
		
		// Store current sample
		m_buffer.write(inSum);

		// Calculate output
		const float outWet = gain * wetModulatedSmooth * delayedSample;
		const float dry = gain * (1.0f - wetModulatedSmooth);

		// Apply wer/dry
		const auto outLeft  = dry * inLeft + outWet;
		const auto outRight = dry * inRight - outWet;

		leftChannel[sample] = outLeft;
		rightChannel[sample] = outRight;

		// Update stereo width envelopes
		m_envelopeMid.process(outLeft + outRight);
		m_envelopeSide.process(outLeft - outRight);

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

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::Delay],				paramsNames[ParamsName::Delay],				NormalisableRange<float>( MINIMUM_DELAY_TIME_MS, MAXIMUM_DELAY_TIME_MS, 0.1f, 0.7f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::Width],				paramsNames[ParamsName::Width],				NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::Color],				paramsNames[ParamsName::Color],				NormalisableRange<float>(-100.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::HP],					paramsNames[ParamsName::HP],				NormalisableRange<float>(  20.0f, 440.0f,  1.0f, 1.0f), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::Dynamic],			paramsNames[ParamsName::Dynamic],			NormalisableRange<float>(-100.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::DynamicSpeed],		paramsNames[ParamsName::DynamicSpeed],		NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::ModulationDepth],	paramsNames[ParamsName::ModulationDepth],	NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::ModulationSpeed],	paramsNames[ParamsName::ModulationSpeed],	NormalisableRange<float>(   0.0f,  10.0f, 0.01f, 0.5f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[ParamsName::Volume],				paramsNames[ParamsName::Volume],			NormalisableRange<float>( -18.0f,  18.0f,  1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonoToStereoAudioProcessor();
}
