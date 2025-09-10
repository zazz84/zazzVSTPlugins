/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string MultiAllPassAudioProcessor::paramsNames[] =		{ "Frequency", "Style", "Intensity", "Volume" };
const std::string MultiAllPassAudioProcessor::labelNames[] =		{ "Frequency", "Style", "Intensity", "Volume" };
const std::string MultiAllPassAudioProcessor::paramsUnitNames[] =	{ " Hz",		"",		"",			 " dB" };

//==============================================================================
MultiAllPassAudioProcessor::MultiAllPassAudioProcessor()
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
	intensityParameter = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);
}

MultiAllPassAudioProcessor::~MultiAllPassAudioProcessor()
{
}

//==============================================================================
const juce::String MultiAllPassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiAllPassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiAllPassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiAllPassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiAllPassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiAllPassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiAllPassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiAllPassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiAllPassAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiAllPassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiAllPassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)(sampleRate);

	m_secondOrderAllPassMulti[0].init(sr);
	m_secondOrderAllPassMulti[1].init(sr);

	m_frequencySmoother.init(sr);
	m_styleSmoother.init(sr);

	m_frequencySmoother.set(2.0f);
	m_styleSmoother.set(2.0f);
}

void MultiAllPassAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiAllPassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MultiAllPassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto frequency = std::fminf((float)(getSampleRate() * (0.5 * 0.9)), frequencyParameter->load());
	const auto style = 0.01f + 2.0f * std::powf(0.01f * styleParameter->load(), 2.0f);
	const auto intensity = intensityParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	if (channels == 1)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);

		auto& secondOrderAllPass = m_secondOrderAllPassMulti[0];
		
		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];

			const float frequencySmooth = m_frequencySmoother.process(frequency);
			const float styleSmooth = m_styleSmoother.process(style);

			secondOrderAllPass.set(frequencySmooth, styleSmooth, intensity);

			channelBuffer[sample] = gain * secondOrderAllPass.process(in);
		}
	}
	else
	{
		// Channel pointers
		auto* channelBufferLeft = buffer.getWritePointer(0);
		auto* channelBufferRight = buffer.getWritePointer(1);

		auto& secondOrderAllPassLeft = m_secondOrderAllPassMulti[0];
		auto& secondOrderAllPassRight = m_secondOrderAllPassMulti[1];

		for (int sample = 0; sample < samples; sample++)
		{
			const float inLeft = channelBufferLeft[sample];
			const float inRight = channelBufferRight[sample];

			const float frequencySmooth = m_frequencySmoother.process(frequency);
			const float styleSmooth = m_styleSmoother.process(style);

			secondOrderAllPassLeft.set(frequencySmooth, styleSmooth, intensity);
			secondOrderAllPassRight.set(frequencySmooth, styleSmooth, intensity);

			channelBufferLeft[sample] = gain * secondOrderAllPassLeft.process(inLeft);
			channelBufferRight[sample] = gain * secondOrderAllPassRight.process(inRight);
		}
	}
}

//==============================================================================
bool MultiAllPassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiAllPassAudioProcessor::createEditor()
{
    return new MultiAllPassAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MultiAllPassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MultiAllPassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiAllPassAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 0.3f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(          0.0f,        100.0f,  1.0f, 1.0f),  50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(          1.0f, N_ALL_PASS_SO,  1.0f, 1.0f),  25.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(        -12.0f,         12.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("Button1", "Button1", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("Button2", "Button2", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiAllPassAudioProcessor();
}
