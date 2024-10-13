/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string CombFilterAudioProcessor::paramsNames[] = { "Frequency", "Stages", "LowCut", "HighCut", "Mix", "Volume" };

//==============================================================================
CombFilterAudioProcessor::CombFilterAudioProcessor()
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
	stagesParameter    = apvts.getRawParameterValue(paramsNames[1]);
	lowCutParameter    = apvts.getRawParameterValue(paramsNames[2]);
	highCutParameter   = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[5]);
}

CombFilterAudioProcessor::~CombFilterAudioProcessor()
{
}

//==============================================================================
const juce::String CombFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CombFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CombFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CombFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CombFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CombFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CombFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CombFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CombFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void CombFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CombFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const auto delay = (int)((float)sampleRate / (2.0f * (float)FREQUENY_MIN));

	for (int i = 0; i < STAGES_MAX; i++)
	{
		m_combFilter[i][0].init(delay);
		m_combFilter[i][1].init(delay);
	}

	const int sr = (int)(sampleRate);

	m_lowCutFilter[0].init(sr);
	m_lowCutFilter[1].init(sr);
	m_highCutFilter[0].init(sr);
	m_highCutFilter[1].init(sr);
}

void CombFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CombFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CombFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto frequency = frequencyParameter->load();
	const int stages = (int)(std::round(stagesParameter->load()));
	const auto lowCut = lowCutParameter->load();
	const auto highCut = highCutParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto mixInverse = 1.0f - mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto delay = (float)getSampleRate() / (2.0f * frequency);
	
	for (int channel = 0; channel < channels; ++channel)
	{
		auto& lowCutFilter = m_lowCutFilter[channel];
		auto& highCutFilter = m_highCutFilter[channel];

		lowCutFilter.setHighPass(lowCut, 1.0f);
		highCutFilter.setLowPass(highCut, 1.0f);

		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		for (int i = 0; i < stages; i++)
		{
			m_combFilter[i][channel].set(delay);
		}

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];
			
			float out = in;
			
			for (int stage = 0; stage < stages; stage++)
			{
				out = m_combFilter[stage][channel].process(out);
			}

			out = lowCutFilter.processDF1(out);
			out = highCutFilter.processDF1(out);

			channelBuffer[sample] = volume * (mixInverse * in + mix * out);
		}
	}
}

//==============================================================================
bool CombFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CombFilterAudioProcessor::createEditor()
{
    return new CombFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void CombFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void CombFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout CombFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>((float)FREQUENY_MIN, (float)FREQUENY_MAX, 1.0f, 0.3f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   1.0f, (float)STAGES_MAX, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>((float)FREQUENY_MIN, 20000.0f, 1.0f, 0.3f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>((float)FREQUENY_MIN, 20000.0f, 1.0f, 0.3f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CombFilterAudioProcessor();
}
