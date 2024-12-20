/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string GuitarAmpAudioProcessor::paramsNames[] = { "Gain", "Bass", "Mid", "Treble", "Volume" };
const std::string GuitarAmpAudioProcessor::paramsUnitNames[] = { " dB", " dB", " dB", "dB", " dB" };

//==============================================================================
GuitarAmpAudioProcessor::GuitarAmpAudioProcessor()
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
	gainParameter = apvts.getRawParameterValue(paramsNames[0]);
	bassParameter = apvts.getRawParameterValue(paramsNames[1]);
	midParameter = apvts.getRawParameterValue(paramsNames[2]);
	trebleParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

GuitarAmpAudioProcessor::~GuitarAmpAudioProcessor()
{
}

//==============================================================================
const juce::String GuitarAmpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GuitarAmpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GuitarAmpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GuitarAmpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GuitarAmpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GuitarAmpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GuitarAmpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GuitarAmpAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GuitarAmpAudioProcessor::getProgramName (int index)
{
    return {};
}

void GuitarAmpAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GuitarAmpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_preampEQ[0].init(sr);
	m_preampEQ[1].init(sr);

	m_preEQ[0].init(sr);
	m_preEQ[1].init(sr);

	m_drivePreEQ[0].init(sr);
	m_drivePreEQ[1].init(sr);

	m_drivePostEQ[0].init(sr);
	m_drivePostEQ[1].init(sr);

	m_drivePreEQ[0].setLowShelf(440.0f, 0.707f, 6.0f);
	m_drivePreEQ[1].setLowShelf(440.0f, 0.707f, 6.0f);

	m_drivePostEQ[0].setLowShelf(440.0f, 0.707f, -6.0f);
	m_drivePostEQ[1].setLowShelf(440.0f, 0.707f, -6.0f);

	m_powerAmp[0].init(sr);
	m_powerAmp[1].init(sr);

	const int size = (int)(sampleRate / (2.0 * 880.0));
	m_combFilter[0].init(size);
	m_combFilter[1].init(size);

	m_combFilter[0].set(0.5f, size);
	m_combFilter[1].set(0.5f, size);

	m_combHPFilter[0].init(sr);
	m_combHPFilter[1].init(sr);

	m_combLPFilter[0].init(sr);
	m_combLPFilter[1].init(sr);

	m_combHPFilter[0].setHighPass(1600.0f, 1.0f);
	m_combHPFilter[1].setHighPass(1600.0f, 1.0f);

	m_combLPFilter[0].setLowPass(3000.0f, 1.0f);
	m_combLPFilter[1].setLowPass(3000.0f, 1.0f);

	m_speakerCabinetSimulation[0].init(sr);
	m_speakerCabinetSimulation[1].init(sr);
}

void GuitarAmpAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GuitarAmpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GuitarAmpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto gain = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto bass = bassParameter->load();
	const auto mid = midParameter->load();
	const auto treble = trebleParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//Filters references
		auto& preEQ = m_preEQ[channel];
		auto& preampEQ = m_preampEQ[channel];
		auto& drivePreEQ = m_drivePreEQ[channel];
		auto& drivePostEQ = m_drivePostEQ[channel];
		auto& powerAmp = m_powerAmp[channel];
		auto& comb = m_combFilter[channel];
		auto& combHPFilter = m_combHPFilter[channel];
		auto& combLPFilter = m_combLPFilter[channel];
		auto& speakerCabinetSimulation = m_speakerCabinetSimulation[channel];


		// Set filters
		preampEQ.set(300.0f, 1800.0f, bass, mid, treble);

		for (int sample = 0; sample < samples; sample++)
		{
			// In
			float out = channelBuffer[sample];

			// PreEQ
			out = preEQ.process(out);

			// Preamp EQ
			out = preampEQ.process(out);

			// Preamp drive
			out = drivePreEQ.processDF1(out);
			out = Waveshapers::Reciprocal(gain * out, 3.0f);
			out = drivePostEQ.processDF1(out);

			// Poweramp
			out = powerAmp.process(out);
			
			// Cabinet - Comb filter
			float combOut = combHPFilter.processDF1(combLPFilter.processDF1(comb.process(out)));
			out = 0.7f * out + 0.3f * combOut;

			//Cabinet
			out = speakerCabinetSimulation.process(out);
			
			channelBuffer[sample] = out;
		}
	}

	buffer.applyGain(volume);
}

//==============================================================================
bool GuitarAmpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GuitarAmpAudioProcessor::createEditor()
{
    return new GuitarAmpAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void GuitarAmpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void GuitarAmpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout GuitarAmpAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	constexpr float GAIN_MIN = -24.0f;
	constexpr float GAIN_MAX = 24.0f;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(GAIN_MIN, GAIN_MAX, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(GAIN_MIN, GAIN_MAX, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(GAIN_MIN, GAIN_MAX, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(GAIN_MIN, GAIN_MAX, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(GAIN_MIN, GAIN_MAX, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GuitarAmpAudioProcessor();
}
