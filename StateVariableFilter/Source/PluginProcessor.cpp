/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string StateVariableFilterAudioProcessor::paramsNames[] = { "Frequency", "Q", "Mix", "Volume" };

//==============================================================================
StateVariableFilterAudioProcessor::StateVariableFilterAudioProcessor()
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
	QParameter         = apvts.getRawParameterValue(paramsNames[1]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[2]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[3]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LP"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("HP"));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("BP"));
}

StateVariableFilterAudioProcessor::~StateVariableFilterAudioProcessor()
{
}

//==============================================================================
const juce::String StateVariableFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StateVariableFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StateVariableFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StateVariableFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StateVariableFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StateVariableFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StateVariableFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StateVariableFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StateVariableFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void StateVariableFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StateVariableFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	int sr = (int)sampleRate;
	m_filter[0].init(sr);
	m_filter[1].init(sr);
}

void StateVariableFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StateVariableFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void StateVariableFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();

	// Get params
	const auto frequency = frequencyParameter->load();
	const auto Q = QParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& filter = m_filter[channel];
		
		if (button1)
		{
			filter.setLowPass(frequency, Q);

			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processLowPass(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
		else if (button2)
		{
			filter.setHighPass(frequency, Q);
			
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processHighPass(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
		else
		{	
			filter.setBandPass(frequency, Q);

			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processBandPass(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
	}
}

//==============================================================================
bool StateVariableFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StateVariableFilterAudioProcessor::createEditor()
{
    return new StateVariableFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void StateVariableFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void StateVariableFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout StateVariableFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 0.3f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(          0.1f,          10.0, 0.01f, 1.0f),   0.7f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(          0.0f,           1.0, 0.01f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(        -18.0f,         18.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("LP", "LP", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("HP", "HP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("BP", "BP", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StateVariableFilterAudioProcessor();
}
