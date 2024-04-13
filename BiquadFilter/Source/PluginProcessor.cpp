/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string BiquadFilterAudioProcessor::paramsNames[] = { "Frequency", "Gain", "Q", "Mix", "Volume" };

//==============================================================================
BiquadFilterAudioProcessor::BiquadFilterAudioProcessor()
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
	gainParameter      = apvts.getRawParameterValue(paramsNames[1]);
	QParameter         = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LP"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("HP"));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("BP1"));
	button4Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("BP2"));
	button5Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("N"));
	button6Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("P"));
	button7Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LS"));
	button8Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("HS"));

	button9Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF1"));
	button10Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF2"));
	button11Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF1T"));
	button12Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF2T"));
}

BiquadFilterAudioProcessor::~BiquadFilterAudioProcessor()
{
}

//==============================================================================
const juce::String BiquadFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BiquadFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BiquadFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BiquadFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BiquadFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BiquadFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BiquadFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BiquadFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BiquadFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void BiquadFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BiquadFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	int sr = (int)sampleRate;
	m_filter[0].init(sr);
	m_filter[1].init(sr);
}

void BiquadFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BiquadFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BiquadFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();
	const auto button4 = button4Parameter->get();
	const auto button5 = button5Parameter->get();
	const auto button6 = button6Parameter->get();
	const auto button7 = button7Parameter->get();
	
	const auto button9 = button9Parameter->get();
	const auto button10 = button10Parameter->get();
	const auto button11 = button11Parameter->get();


	// Get params
	const auto frequency = frequencyParameter->load();
	const auto gain = gainParameter->load();
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
			filter.setLowPass(frequency, Q);
		else if(button2)
			filter.setHighPass(frequency, Q);
		else if(button3)
			filter.setBandPassSkirtGain(frequency, Q);
		else if(button4)
			filter.setBandPassPeakGain(frequency, Q);
		else if(button5)
			filter.setNotch(frequency, Q);
		else if(button6)
			filter.setPeak(frequency, Q, gain);
		else if(button7)
			filter.setLowShelf(frequency, Q, gain);
		else
			filter.setHighShelf(frequency, Q, gain);


		if (button9)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processDF1(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
		else if (button10)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processDF2(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
		else if (button11)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processDF1T(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
		else
		{
			for (int sample = 0; sample < samples; sample++)
			{
				const float in = channelBuffer[sample];
				const float inFiltered = filter.processDF2T(in);
				channelBuffer[sample] = volume * ((1.0f - mix) * in + inFiltered * mix);
			}
		}
	}
}

//==============================================================================
bool BiquadFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BiquadFilterAudioProcessor::createEditor()
{
    return new BiquadFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BiquadFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BiquadFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BiquadFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 0.3f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(        -18.0f,         18.0f, 0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(          0.1f,          10.0, 0.01f, 1.0f),   0.7f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(          0.0f,           1.0, 0.01f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(        -18.0f,         18.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("LP", "LP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("HP", "HP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("BP1", "BP1", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("BP2", "BP2", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("N", "N", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("P", "P", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("LS", "LS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("HS", "HS", false));

	layout.add(std::make_unique<juce::AudioParameterBool>("DF1", "DF1", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF2", "DF2", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF1T", "DF1T", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF2T", "DF2T", true));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BiquadFilterAudioProcessor();
}
