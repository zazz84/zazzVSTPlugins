/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string FDNReverbAudioProcessor::paramsNames[] = { "Size", "Resonance", "Absorbrion", "Color", "Width", "Mix", "Volume" };
const float FDNReverbAudioProcessor::MAXIMUM_DELAY_TIME = 0.15f;

//==============================================================================
FDNReverbAudioProcessor::FDNReverbAudioProcessor()
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
	sizeParameter      = apvts.getRawParameterValue(paramsNames[0]);
	resonanceParameter = apvts.getRawParameterValue(paramsNames[1]);
	dampingParameter   = apvts.getRawParameterValue(paramsNames[2]);
	colorParameter     = apvts.getRawParameterValue(paramsNames[3]);
	widthParameter     = apvts.getRawParameterValue(paramsNames[4]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[5]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[6]);
}

FDNReverbAudioProcessor::~FDNReverbAudioProcessor()
{
}

//==============================================================================
const juce::String FDNReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FDNReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FDNReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FDNReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FDNReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FDNReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FDNReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FDNReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FDNReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int maximumSize = (int)(MAXIMUM_DELAY_TIME * sampleRate);
	const int sr = (int)sampleRate;
	
	for (int channel = 0; channel < 2; channel++)
	{
		m_lowShelf[channel].init(sr);
		m_highShelf[channel].init(sr);

		for (int buffer = 0; buffer < DELAY_LINES_COUNT; buffer++)
		{
			m_buffer[channel][buffer].init(maximumSize);
			m_filter[channel][buffer].init(sr);
		}
	}

	/*for (int buffer = 0; buffer < DELAY_LINES_COUNT; buffer++)
	{
		//Get distance
		const float delayTime = MAXIMUM_DELAY_TIME * m_primeNumbers[buffer] / (float)m_primeNumbers[DELAY_LINES_COUNT - 1];
		const float distance = delayTime * 343.0f;
		constexpr auto a = 3.0f;
		m_bGain[buffer] = a / (distance + a);
	}*/
}

void FDNReverbAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FDNReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FDNReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto size = sizeParameter->load();
	const auto resonance = resonanceParameter->load();
	const auto damping = dampingParameter->load();
	const auto color = colorParameter->load();
	const auto width = widthParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto mixInverse = 1.0f - mix;
	const auto mixAdjustedGain = 0.25f * mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	const auto delayTime = 0.001f + (MAXIMUM_DELAY_TIME - 0.001f) * size;
	const auto feedback = 0.95f * resonance;
	const auto gain = color * 6.0f;
	const auto widthFactor = 1.0f - width * 0.015f;	
	const float primeNumbersNormalize = 1.0f / (float)m_primeNumbers[15];
	const float samplesMultiplier = delayTime * sampleRate * primeNumbersNormalize;
	const auto damping3 = 0.01f * damping * damping * damping;

	for (int buffer = 0; buffer < DELAY_LINES_COUNT; buffer++)
	{
		// Set delay time
		const float delayTime = (float)m_primeNumbers[buffer] * samplesMultiplier;
		m_buffer[0][buffer].setSize((int)delayTime);
		m_buffer[1][buffer].setSize((int)(widthFactor * delayTime));

		//Set bGain
		const float time = MAXIMUM_DELAY_TIME * (float)m_primeNumbers[buffer] * primeNumbersNormalize;
		const float distance = time * 343.0f;
		constexpr auto a = 10.0f;
		m_bGain[buffer] = a / (distance + a);

		// Set filter
		const auto frequency = 16000.0f * std::expf(-damping3 * distance);
		m_filter[0][buffer].set(frequency);
		m_filter[1][buffer].set(frequency);
	}

	// Process buffer
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& lowShelf = m_lowShelf[channel];
		auto& highShelf = m_highShelf[channel];

		lowShelf.setLowShelf(660.0f, 0.4f, -gain);
		highShelf.setHighShelf(660.0f, 0.4f, gain);

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];

			// Add color
			const float inColor = lowShelf.processDF1(highShelf.processDF1(in));

			//Read samples
			float out = 0.0f;
			for (int buffer = 0; buffer < DELAY_LINES_COUNT; buffer++)
			{
				m_tmp[buffer] = m_buffer[channel][buffer].read();

				// Filter
				m_tmp[buffer] = m_filter[channel][buffer].process(m_tmp[buffer]);

				out += m_tmp[buffer];
			}

			// FWHT
			FWHT(m_tmp);

			// Writte sample
			for (int buffer = 0; buffer < DELAY_LINES_COUNT; buffer++)
			{
				m_buffer[channel][buffer].writeSample(m_bGain[buffer] * inColor + feedback * m_tmp[buffer]);
				//m_buffer[channel][buffer].writeSample(inColor + feedback * m_tmp[buffer]);
			}
			
			channelBuffer[sample] = volume * (mixInverse * in + mixAdjustedGain * out);
		}
	}
}

//==============================================================================
bool FDNReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FDNReverbAudioProcessor::createEditor()
{
    return new FDNReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void FDNReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void FDNReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout FDNReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( -18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FDNReverbAudioProcessor();
}
