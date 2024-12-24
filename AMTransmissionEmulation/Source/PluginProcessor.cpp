/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string AMTransmissionEmulationAudioProcessor::paramsNames[] = { "Modulation Depth", "Release", "Tune", "Noise", "Volume" };
const std::string AMTransmissionEmulationAudioProcessor::paramsUnitNames[] = { " %", " ms", " Hz", " dB", " dB" };

//==============================================================================
AMTransmissionEmulationAudioProcessor::AMTransmissionEmulationAudioProcessor()
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
	modulatioDepthParameter = apvts.getRawParameterValue(paramsNames[0]);
	releaseParameter = apvts.getRawParameterValue(paramsNames[1]);
	tuneParameter = apvts.getRawParameterValue(paramsNames[2]);
	noiseParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);
}

AMTransmissionEmulationAudioProcessor::~AMTransmissionEmulationAudioProcessor()
{
}

//==============================================================================
const juce::String AMTransmissionEmulationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AMTransmissionEmulationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AMTransmissionEmulationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AMTransmissionEmulationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AMTransmissionEmulationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AMTransmissionEmulationAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AMTransmissionEmulationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AMTransmissionEmulationAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AMTransmissionEmulationAudioProcessor::getProgramName (int index)
{
    return {};
}

void AMTransmissionEmulationAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AMTransmissionEmulationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	const int srOversample = OVERSAMPLING_RATIO * sr;

	// No oversample version
	m_oscillator[0].init(sr);
	m_oscillator[1].init(sr);

	m_DCFilter[0].init(sr);
	m_DCFilter[1].init(sr);

	m_DCFilter[0].set(200.0f);
	m_DCFilter[1].set(200.0f);

	m_diodeDetector[0].init(sr);
	m_diodeDetector[1].init(sr);

	constexpr float releaseTime = 1.0f;
	m_diodeDetector[0].set(0.0f, releaseTime);
	m_diodeDetector[1].set(0.0f, releaseTime);

	m_lowPassFilter[0].init(sr);
	m_lowPassFilter[1].init(sr);

	m_lowPassFilter[0].set(8000.0f);
	m_lowPassFilter[1].set(8000.0f);

	m_lowPassPostFilter[0].init(sr);
	m_lowPassPostFilter[1].init(sr);

	m_lowPassPostFilter[0].set(4000.0f);
	m_lowPassPostFilter[1].set(4000.0f);

	m_tuningFilter[0].init(sr);
	m_tuningFilter[1].init(sr);

	m_noiseFilter[0].init(sr);
	m_noiseFilter[1].init(sr);

	m_tuningFrequencySmoother[0].init(sr);
	m_tuningFrequencySmoother[1].init(sr);

	m_tuningFrequencySmoother[0].set(2.0f);
	m_tuningFrequencySmoother[1].set(2.0f);

	// Oversample version
	/*m_oscillator[0].init(srOversample);
	m_oscillator[1].init(srOversample);

	m_DCFilter[0].init(srOversample);
	m_DCFilter[1].init(srOversample);

	m_DCFilter[0].set(200.0f);
	m_DCFilter[1].set(200.0f);

	m_oversampling[0].init(sr, OVERSAMPLING_RATIO, samplesPerBlock);
	m_oversampling[1].init(sr, OVERSAMPLING_RATIO, samplesPerBlock);

	m_diodeDetector[0].init(srOversample);
	m_diodeDetector[1].init(srOversample);

	constexpr float releaseTime = 0.1f;
	m_diodeDetector[0].set(0.0f, releaseTime);
	m_diodeDetector[1].set(0.0f, releaseTime);*/
}

void AMTransmissionEmulationAudioProcessor::releaseResources()
{
	m_oversampling[0].release();
	m_oversampling[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AMTransmissionEmulationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AMTransmissionEmulationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Misc constants
	const int channels = buffer.getNumChannels();
	const int samples = buffer.getNumSamples();
	const float sampleRate = (float)getSampleRate();
	const float sampleRateHalf = 0.5f * sampleRate;
	
	const auto modulationDepth = 0.01f * modulatioDepthParameter->load();
	const auto release = releaseParameter->load();
	const auto tune = tuneParameter->load();
	const auto noiseGain = juce::Decibels::decibelsToGain(noiseParameter->load());
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Misc constants
	const float carrierFrequency = 0.8f * sampleRateHalf;
	const int oversampleSamples = OVERSAMPLING_RATIO * samples;
	const float oversampleSampleRate = OVERSAMPLING_RATIO * sampleRate;
	const float tuningFrequency = std::fmaxf(20000.0f, std::fminf(sampleRateHalf, carrierFrequency + tune));

	for (int channel = 0; channel < channels; channel++)
	{
		float* channelData = buffer.getWritePointer(channel);

		auto& oversampling = m_oversampling[channel];
		auto& DCFilter = m_DCFilter[channel];
		auto& diodeDetector = m_diodeDetector[channel];
		auto& oscillator = m_oscillator[channel];
		auto& lowPassFilter = m_lowPassFilter[channel];
		auto& lowPassPostFitler = m_lowPassPostFilter[channel];
		auto& tuningFilter = m_tuningFilter[channel];
		auto& noiseFilter = m_noiseFilter[channel];
		auto& tuningFrequencySmoother = m_tuningFrequencySmoother[channel];

		diodeDetector.set(0.0f, release);
		//tuningFilter.setBandPassPeakGain(tuningFilterFrequency, 50.0f);
		//noiseFilter.setNotch(tuningFilterFrequency, 12.0f);

		// No oversample version
		oscillator.set(carrierFrequency);

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelData[sample];

			float out = in;

			// Limit frequency range
			//out = lowPassFilter.process(out);

			// Apply AM modulation
			out = (1.0f + modulationDepth * out) *  oscillator.process();

			// Smooth tuning frequency
			float const tuningFrequencySmooth = tuningFrequencySmoother.process(tuningFrequency);
			tuningFilter.setBandPassPeakGain(tuningFrequencySmooth, 50.0f);
			noiseFilter.setNotch(tuningFrequencySmooth, 12.0f);
			
			// Generate noise
			//const float noise = noiseGain * noiseFilter.processDF1(2.0f * m_noise.process() - 1.0f);
			const float noise = noiseGain * (2.0f * m_noise.process() - 1.0f);
			
			// Add noise
			out = out + noise;
			
			// Tune to carrier
			out = tuningFilter.processDF1(out);
			
			// Diode detector
			if (release != 0)
			{
				out = diodeDetector.process(out);
			}

			// Remove DC offset
			out = DCFilter.process(out);

			// Post filter
			//out = lowPassPostFitler.process(out);

			// Writte to output
			channelData[sample] = out;
		}


		// Oversample version
		/*oscillator.set(200000.0f);

		// Oversample
		oversampling.oversample(channelData);

		float* oversampleBuffer = oversampling.getOversampeBuffer();

		// Process oversampeled data
		for (int sample = 0; sample < oversampleSamples; sample++)
		{
			const float in = oversampleBuffer[sample];
			
			float out = in;

			// Apply AM modulation
			out = (1.0f + modulationDepth * out) * oscillator.process();

			// Diode detector
			out = diodeDetector.process(out);
			
			// Remove DC offset
			out = DCFilter.process(out);

			// Writte to output
			oversampleBuffer[sample] = out;
		}

		// Downsample
		oversampling.downsample(channelData);*/
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool AMTransmissionEmulationAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AMTransmissionEmulationAudioProcessor::createEditor()
{
    return new AMTransmissionEmulationAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void AMTransmissionEmulationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AMTransmissionEmulationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AMTransmissionEmulationAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 0.0f, 200.0f,  1.0f, 1.0f), 80.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 0.0f, 10.0f, 0.00001f, 0.2f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -20000.0f, 20000.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -60.0f,  0.0f, 1.0f, 1.0f), -60.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AMTransmissionEmulationAudioProcessor();
}
