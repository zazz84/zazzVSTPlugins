/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string NoiseGeneratorAudioProcessor::paramsNames[] =		{ "Type",	"Density",	"Wet",	"Dry" };
const std::string NoiseGeneratorAudioProcessor::labelNames[]  =		{ "Type",	"Density",	"Wet",	"Dry" };
const std::string NoiseGeneratorAudioProcessor::paramsUnitNames[] = { "",		" %",		" dB",	" dB" };

//==============================================================================
NoiseGeneratorAudioProcessor::NoiseGeneratorAudioProcessor()
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
	typeParameter		= apvts.getRawParameterValue(paramsNames[0]);
	densityParameter	= apvts.getRawParameterValue(paramsNames[1]);
	wetParameter		= apvts.getRawParameterValue(paramsNames[2]);
	dryParameter		= apvts.getRawParameterValue(paramsNames[3]);
}

NoiseGeneratorAudioProcessor::~NoiseGeneratorAudioProcessor()
{
}

//==============================================================================
const juce::String NoiseGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoiseGeneratorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGeneratorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGeneratorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoiseGeneratorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoiseGeneratorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoiseGeneratorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoiseGeneratorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoiseGeneratorAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoiseGeneratorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoiseGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	//const int sr = (int)sampleRate;
}

void NoiseGeneratorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoiseGeneratorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NoiseGeneratorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Prevent denormals for this function scope
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto type   = (int)typeParameter->load();
	const auto density = 0.01f * densityParameter->load();
	const auto densityScaled = density * density * density;

	// Gain compenstaion for velvet noise
	const auto gainCompensation = 25.6706f - 54.89829f * density + 24.90157f * density * density;

	const auto wetdB = type == 1 ? wetParameter->load() : gainCompensation + wetParameter->load();
	const auto drydB = dryParameter->load();

	const auto wet = wetdB > -60.0f ? juce::Decibels::decibelsToGain(wetdB) : 0.0f;
	const auto dry = drydB > -60.0f ? juce::Decibels::decibelsToGain(drydB) : 0.0f;

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);

		if (type == 1)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float noise = m_whiteNoiseGenerator.process11();
				channelBuffer[sample] = dry * in + wet * noise;
			}
		}
		else if (type == 2)
		{
			m_velvetNoiseGenerator.set(densityScaled);

			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float noise = m_velvetNoiseGenerator.process11();
				channelBuffer[sample] = dry * in + wet * noise;
			}
		}
		else if (type == 3)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float noise = m_pinkNoiseGenerator.process();
				channelBuffer[sample] = dry * in + wet * noise;
			}
		}
	}
}

//==============================================================================
bool NoiseGeneratorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoiseGeneratorAudioProcessor::createEditor()
{
    return new NoiseGeneratorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void NoiseGeneratorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void NoiseGeneratorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout NoiseGeneratorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   3.0f,  1.0f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  70.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -60.0f,  12.0f,  1.0f, 1.0f), -24.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -60.0f,  12.0f,  1.0f, 1.0f), -60.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseGeneratorAudioProcessor();
}