/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string DistortionAudioProcessor::paramsNames[] = { "Drive", "Asymetry", "Foldback", "Cutoff", "Resonance", "Dynamics", "Mix", "Volume" };

//==============================================================================
DistortionAudioProcessor::DistortionAudioProcessor()
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
	driveParameter     = apvts.getRawParameterValue(paramsNames[0]);
	asymetryParameter  = apvts.getRawParameterValue(paramsNames[1]);
	foldbackParameter  = apvts.getRawParameterValue(paramsNames[2]);
	frequencyParameter = apvts.getRawParameterValue(paramsNames[3]);
	resonanceParameter = apvts.getRawParameterValue(paramsNames[4]);	
	dynamicsParameter  = apvts.getRawParameterValue(paramsNames[5]);	
	mixParameter       = apvts.getRawParameterValue(paramsNames[6]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[7]);
}

DistortionAudioProcessor::~DistortionAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	m_lowPassFilter[0].init(sr);
	m_lowPassFilter[1].init(sr);
	
	m_highPassFilter[0].init(sr);
	m_highPassFilter[1].init(sr);
	m_highPassFilter[0].setHighPass(10.0f, 0.5f);
	m_highPassFilter[1].setHighPass(10.0f, 0.5f);

	m_clipper[0].init(sr);
	m_clipper[1].init(sr);

	m_gainComponesation[0].init(sr);
	m_gainComponesation[1].init(sr);

	m_exponentialWaveShaper[0].init(sr);
	m_exponentialWaveShaper[1].init(sr);
}

void DistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto drive = driveParameter->load();
	const auto asymetry = asymetryParameter->load();
	const auto frequency = frequencyParameter->load();
	const auto resonance = resonanceParameter->load() * 4.0f;	
	const auto dynamics = dynamicsParameter->load();
	const auto foldback = foldbackParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	const auto mix = mixParameter->load();

	// Mics constants
	const float mixInverse = 1.0f - mix;
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();
	const float threshold = juce::Decibels::decibelsToGain(foldback);

	for (int channel = 0; channel < channels; channel++)
	{
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		auto& lowPassFilter = m_lowPassFilter[channel];
		auto& hightPassFilter = m_highPassFilter[channel];
		auto& exponentialWaveShaper = m_exponentialWaveShaper[channel];
		auto& foldbackWaveShaper = m_foldBackWaveShaper[channel];
		auto& clipper = m_clipper[channel];
		auto& gainCompensation = m_gainComponesation[channel];
		
		// Set DSPs
		lowPassFilter.setLowPass(frequency, 0.707f + resonance);
		exponentialWaveShaper.set(drive, asymetry);
		foldbackWaveShaper.set(threshold);
		gainCompensation.set(dynamics);

		for (int sample = 0; sample < samples; sample++)
		{
			// Get input
			const float in = channelBuffer[sample];

			// Exponential WaveShaper
			float inProcessed = exponentialWaveShaper.process(in);

			// High pass filter
			inProcessed = hightPassFilter.processDF1(inProcessed);

			// Foldback waveshaper
			inProcessed = foldbackWaveShaper.process(inProcessed);

			// Low pass filter
			inProcessed = lowPassFilter.processDF1(inProcessed);

			// Hard clip
			inProcessed = clipper.process(inProcessed);

			// Apply gain compensation
			inProcessed = gainCompensation.getGainCompensation(in, inProcessed) * inProcessed;

			// Apply volume and mix
			channelBuffer[sample] = volume * (mix * inProcessed + mixInverse * in);
		}
	}
}

//==============================================================================
bool DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionAudioProcessor::createEditor()
{
    return new DistortionAudioProcessorEditor(*this, apvts);
}

//==============================================================================
void DistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void DistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout DistortionAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(-1.0f,1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 0.0f,1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(-60.0f, 0.0f, 1.00f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(40.0f, 20000.0f, 1.0f, 0.4f), 200000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(-36.0f, 36.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionAudioProcessor();
}
