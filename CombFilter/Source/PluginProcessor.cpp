/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const ModernRotarySlider::ParameterDescription CombFilterAudioProcessor::m_parametersDescritpion[] =
{
	{ "Frequency",
	  " Hz",
	  "Frequency" },

	{ "Stages",
	  "",
	  "Stages" },

	{ "Low Cut",
	  " Hz",
	  "Low Cut" },

	{ "High Cut",
	" Hz",
	"High Cut" },

	{ "Mix",
	  " %",
	  "Mix" },

	{ "Volume",
	  " dB",
	  "Volume" }
};

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
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		m_parameters[i] = apvts.getRawParameterValue(m_parametersDescritpion[i].paramName);
	}
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
	const int sr = (int)(sampleRate);

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		for (int i = 0; i < STAGES_MAX; i++)
		{
			m_combFilter[channel][i].init(delay);
			//m_combFilter[channel][i].init(delay, samplesPerBlock);
		}

		m_lowCutFilter[channel].init(sr);
		m_highCutFilter[channel].init(sr);
	}
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
	juce::ScopedNoDenormals noDenormals;

	// Get params
	std::array<float, Parameters::COUNT> parametersValues;
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		parametersValues[i] = m_parameters[i]->load();
	}
	
	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Channels loop constants
	const auto delay = (float)getSampleRate() / (2.0f * parametersValues[Parameters::Frequency]);

	// Samples loop constants
	auto wet = 0.01f * parametersValues[Parameters::Mix];
	auto dry = 1.0f - wet;
	const auto gain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);
	wet *= gain;
	dry *= gain;
	const auto stages = (int)(parametersValues[Parameters::Stages]);
	
	for (int channel = 0; channel < channels; ++channel)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		auto& combFilter = m_combFilter[channel];
		auto& lowCutFilter = m_lowCutFilter[channel];
		auto& highCutFilter = m_highCutFilter[channel];

		constexpr float Q = 1.0f;
		lowCutFilter.setHighPass(parametersValues[Parameters::LowCut], Q);
		highCutFilter.setLowPass(parametersValues[Parameters::HighCut], Q);	

		for (int stage = 0; stage < stages; stage++)
		{
			combFilter[stage].set(delay);
		}

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];
			
			float out = in;
			
			for (int stage = 0; stage < stages; stage++)
			{
				out = combFilter[stage].process(out);
			}

			out = lowCutFilter.processDF1(out);
			out = highCutFilter.processDF1(out);

			channelBuffer[sample] = dry * in + wet * out;
		}

		// Copy input to temp buffer
		/*float* input = channelBuffer;

		for (int stage = 0; stage < stages; stage++)
		{
			auto& comb = combFilter[stage];
			
			float* linearBuffer = comb.getLinearBuffer();

			for (int sample = 0; sample < samples; sample++)
			{
				constexpr float FEEDBACK = 0.5f;	
				linearBuffer[sample] = FEEDBACK * (input[sample] - linearBuffer[sample]);
			}

			comb.moveLinearBufferToCircularBuffer();
			input = linearBuffer;
		}

		for (int sample = 0; sample < samples; sample++)
		{
			// Read comb filters out
			float combOut = input[sample];

			combOut = lowCutFilter.processDF1(combOut);
			combOut = highCutFilter.processDF1(combOut);

			channelBuffer[sample] = dry * channelBuffer[sample] + wet * combOut;
		}*/
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

	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Frequency].paramName, m_parametersDescritpion[Parameters::Frequency].paramName, NormalisableRange<float>((float)FREQUENY_MIN, (float)FREQUENY_MAX, 1.0f, 0.3f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Stages].paramName, m_parametersDescritpion[Parameters::Stages].paramName, NormalisableRange<float>(1.0f, (float)STAGES_MAX, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::LowCut].paramName, m_parametersDescritpion[Parameters::LowCut].paramName, NormalisableRange<float>((float)FREQUENY_MIN, 20000.0f, 1.0f, 0.3f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::HighCut].paramName, m_parametersDescritpion[Parameters::HighCut].paramName, NormalisableRange<float>((float)FREQUENY_MIN, 20000.0f, 1.0f, 0.3f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Mix].paramName, m_parametersDescritpion[Parameters::Mix].paramName, NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(m_parametersDescritpion[Parameters::Volume].paramName, m_parametersDescritpion[Parameters::Volume].paramName, NormalisableRange<float>(-18.0f,  18.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CombFilterAudioProcessor();
}
