/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string VocalCompressorAudioProcessor::paramsNames[] =		{ "Gain",	"Mix", "Volume", "Type" };
const std::string VocalCompressorAudioProcessor::labelNames[] =			{ "Gain",	"Mix", "Volume", "Type" };
const std::string VocalCompressorAudioProcessor::paramsUnitNames[] =	{ " dB",	" %",	" dB",	 "" };

//==============================================================================
VocalCompressorAudioProcessor::VocalCompressorAudioProcessor()
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
	gainParameter   = apvts.getRawParameterValue(paramsNames[0]);
	mixParameter    = apvts.getRawParameterValue(paramsNames[1]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[2]);
	typeParameter	= apvts.getRawParameterValue(paramsNames[3]);
}

VocalCompressorAudioProcessor::~VocalCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String VocalCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VocalCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VocalCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VocalCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VocalCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 1.0;
}

int VocalCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VocalCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VocalCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VocalCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void VocalCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VocalCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = int(sampleRate);

	m_vocalCompressor[0].init(sr);
	m_vocalCompressor[1].init(sr);

	m_vocalCompressorClean[0].init(sr);
	m_vocalCompressorClean[1].init(sr);
}

void VocalCompressorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VocalCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VocalCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto gain = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto mix = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	const auto type = typeParameter->load();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto dry = 1.0f - mix;

	for (int channel = 0; channel < channels; ++channel)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		if (type == 1)
		{
			auto& vocalCompressor = m_vocalCompressor[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				const float in = gain * channelBuffer[sample];

				// Get input peak
				const float inAbs = std::fabsf(in);
				if (inAbs > m_inputMax)
				{
					m_inputMax = inAbs;
				}

				float out = vocalCompressor.process(in);
				out = dry * in + mix * out;

				// Get output peak
				const float outAbs = std::fabsf(out);
				if (outAbs > m_outputMax)
				{
					m_outputMax = outAbs;
				}

				channelBuffer[sample] = volume * out;
			}
		}
		else
		{
			auto& vocalCompressor = m_vocalCompressorClean[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				const float in = gain * channelBuffer[sample];

				// Get input peak
				const float inAbs = std::fabsf(in);
				if (inAbs > m_inputMax)
				{
					m_inputMax = inAbs;
				}

				float out = vocalCompressor.process(in);
				out = dry * in + mix * out;

				// Get output peak
				const float outAbs = std::fabsf(out);
				if (outAbs > m_outputMax)
				{
					m_outputMax = outAbs;
				}

				channelBuffer[sample] = volume * out;
			}
		}
	}
}

//==============================================================================
bool VocalCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VocalCompressorAudioProcessor::createEditor()
{
    return new VocalCompressorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void VocalCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void VocalCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalCompressorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 100.0f, 1.0f,  1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   1.0f,   2.0f, 1.0f,  1.0f),   1.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalCompressorAudioProcessor();
}
