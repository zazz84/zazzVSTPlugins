/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string EnvelopeClonerAudioProcessor::paramsNames[] = { "Dynamics", "Spectrum", "Attack", "Release", "Mix", "Volume" };
const float EnvelopeClonerAudioProcessor::ENVELOPE_MINIMUM = 0.0001f;
const float EnvelopeClonerAudioProcessor::RATIO_LIMIT = juce::Decibels::decibelsToGain(24.0f);

//==============================================================================
EnvelopeClonerAudioProcessor::EnvelopeClonerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
					   .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)
                       )
#endif
{
	dynamicsParameter  = apvts.getRawParameterValue(paramsNames[0]);
	spectrumParameter  = apvts.getRawParameterValue(paramsNames[1]);
	attackParameter    = apvts.getRawParameterValue(paramsNames[2]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[5]);
}

EnvelopeClonerAudioProcessor::~EnvelopeClonerAudioProcessor()
{
}

//==============================================================================
const juce::String EnvelopeClonerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnvelopeClonerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EnvelopeClonerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EnvelopeClonerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EnvelopeClonerAudioProcessor::getTailLengthSeconds() const
{
    return 0.05f;
}

int EnvelopeClonerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EnvelopeClonerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnvelopeClonerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EnvelopeClonerAudioProcessor::getProgramName (int index)
{
    return {};
}

void EnvelopeClonerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EnvelopeClonerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	// Set filters
	
	const int sr = (int)(sampleRate);
	m_lowMidFilter[0].init(sr);
	m_lowMidFilter[1].init(sr);
	m_midHighFilter[0].init(sr);
	m_midHighFilter[1].init(sr);
	m_allPassFilter[0].init(sr);
	m_allPassFilter[1].init(sr);

	m_lowMidFilterSC[0].init(sr);
	m_lowMidFilterSC[1].init(sr);
	m_midHighFilterSC[0].init(sr);
	m_midHighFilterSC[1].init(sr);
	m_allPassFilterSC[0].init(sr);
	m_allPassFilterSC[1].init(sr);

	const float midCrossOverFrequency = 440.0f;
	const float highCrossOverFrequency = 3000.0f;

	m_lowMidFilter[0].setFrequency(midCrossOverFrequency);
	m_lowMidFilter[1].setFrequency(midCrossOverFrequency);
	m_midHighFilter[0].setFrequency(highCrossOverFrequency);
	m_midHighFilter[1].setFrequency(highCrossOverFrequency);
	m_allPassFilter[0].setFrequency(highCrossOverFrequency);
	m_allPassFilter[1].setFrequency(highCrossOverFrequency);

	m_lowMidFilterSC[0].setFrequency(midCrossOverFrequency);
	m_lowMidFilterSC[1].setFrequency(midCrossOverFrequency);
	m_midHighFilterSC[0].setFrequency(highCrossOverFrequency);
	m_midHighFilterSC[1].setFrequency(highCrossOverFrequency);
	m_allPassFilterSC[0].setFrequency(highCrossOverFrequency);
	m_allPassFilterSC[1].setFrequency(highCrossOverFrequency);

	// Set envelope followers

	m_envelopeDetection[0][0].init(sr);
	m_envelopeDetection[0][1].init(sr);
	m_envelopeDetection[0][2].init(sr);
	m_envelopeDetection[1][0].init(sr);
	m_envelopeDetection[1][1].init(sr);
	m_envelopeDetection[1][2].init(sr);

	m_envelopeDetectionSC[0][0].init(sr);
	m_envelopeDetectionSC[0][1].init(sr);
	m_envelopeDetectionSC[0][2].init(sr);
	m_envelopeDetectionSC[1][0].init(sr);
	m_envelopeDetectionSC[1][1].init(sr);
	m_envelopeDetectionSC[1][2].init(sr);

	m_envelopeDetectionFullSpectrum[0].init(sr);
	m_envelopeDetectionFullSpectrum[1].init(sr);
	m_envelopeDetectionFullSpectrumSC[0].init(sr);
	m_envelopeDetectionFullSpectrumSC[1].init(sr);
}

void EnvelopeClonerAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EnvelopeClonerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
	#if JucePlugin_IsMidiEffect
		juce::ignoreUnused(layouts);
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

void EnvelopeClonerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// parameters
	const auto dynamics = dynamicsParameter->load();
	const auto spectrum = spectrumParameter->load();
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	
	// input buffer
	auto mainBus = getBus(true, 0);
	auto mainBuffer = mainBus->getBusBuffer(buffer);

	// sidechain buffer
	auto sideChainBus = getBus(true, 1);

	if (sideChainBus == nullptr || !sideChainBus->isEnabled())
		return;

	auto sideChainBuffer = sideChainBus->getBusBuffer(buffer);
	
	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Filters
		auto& lowMifFilter = m_lowMidFilter[channel];
		auto& midHighFilter = m_midHighFilter[channel];
		auto& allPassFilter = m_allPassFilter[channel];

		auto& lowMifFilterSC = m_lowMidFilterSC[channel];
		auto& midHighFilterSC = m_midHighFilterSC[channel];
		auto& allPassFilterSC = m_allPassFilterSC[channel];

		// Envelopes
		auto& envelopeDetectionFullSpectrum = m_envelopeDetectionFullSpectrum[channel];
		auto& envelopeDetectionFullSpectrumSC = m_envelopeDetectionFullSpectrumSC[channel];

		auto& envelopeDetectionLow = m_envelopeDetection[channel][0];
		auto& envelopeDetectionMid = m_envelopeDetection[channel][1];
		auto& envelopeDetectionHigh = m_envelopeDetection[channel][2];

		auto& envelopeDetectionLowSC = m_envelopeDetectionSC[channel][0];
		auto& envelopeDetectionMidSC = m_envelopeDetectionSC[channel][1];
		auto& envelopeDetectionHighSC = m_envelopeDetectionSC[channel][2];

		envelopeDetectionFullSpectrum.setCoef(attack, release);
		envelopeDetectionFullSpectrumSC.setCoef(attack, release);

		envelopeDetectionLow.setCoef(attack, release);
		envelopeDetectionMid.setCoef(attack, release);
		envelopeDetectionHigh.setCoef(attack, release);

		envelopeDetectionLowSC.setCoef(attack, release);
		envelopeDetectionMidSC.setCoef(attack, release);
		envelopeDetectionHighSC.setCoef(attack, release);
;
		// Buffers
		auto mainChannelBuffer = mainBuffer.getWritePointer(channel);
		auto sideChainChannelBuffer = sideChainBuffer.getWritePointer(channel);

		for (int sample = 0; sample < samples; ++sample)
		{
			// Input
			const float in = mainChannelBuffer[sample];
			const float inSC = sideChainChannelBuffer[sample];

			// Output envelope calculation
			const float inEnvelope = envelopeDetectionFullSpectrum.process(in);
			const float scEnvelope = envelopeDetectionFullSpectrumSC.process(inSC);
			
			const float gainRatio = fminf(RATIO_LIMIT, (inEnvelope > ENVELOPE_MINIMUM) ? scEnvelope / inEnvelope : 0.0f);

			// Apply gain adjustment to spectrum
			const float gainRatioAdjusted = (gainRatio > 1.0f) ? 1.0f + ((gainRatio - 1.0f) * dynamics) : 1.0f - ((1.0f - gainRatio) * dynamics);

			// Filter to lows, mids and highs
			const float low = lowMifFilter.processLP(in);
			const float midHigh = lowMifFilter.processHP(in);
			const float mid = midHighFilter.processLP(midHigh);
			const float high = midHighFilter.processHP(midHigh);
			const float lowAllPass = allPassFilter.process(low);

			const float lowSC = lowMifFilterSC.processLP(inSC);
			const float midHighSC = lowMifFilterSC.processHP(inSC);
			const float midSC = midHighFilterSC.processLP(midHighSC);
			const float highSC = midHighFilterSC.processHP(midHighSC);
			const float lowAllPassSC = allPassFilterSC.process(lowSC);

			// Get envelope
			float lowEnvelope = envelopeDetectionLow.process(lowAllPass);
			float midEnvelope = envelopeDetectionMid.process(mid);
			float highEnvelope = envelopeDetectionHigh.process(high);

			float lowEnvelopeSC = envelopeDetectionLowSC.process(lowAllPassSC);
			float midEnvelopeSC = envelopeDetectionMidSC.process(midSC);
			float highEnvelopeSC = envelopeDetectionHighSC.process(highSC);

			// Normalized envelopes
			const float inEnvelopeAverage = (lowEnvelope + midEnvelope + highEnvelope) / 3.0f;
			const float scEnvelopeAverage = (lowEnvelopeSC + midEnvelopeSC + highEnvelopeSC) / 3.0f;

			lowEnvelope /= inEnvelopeAverage;
			midEnvelope /= inEnvelopeAverage;
			highEnvelope /= inEnvelopeAverage;

			lowEnvelopeSC /= scEnvelopeAverage;
			midEnvelopeSC /= scEnvelopeAverage;
			highEnvelopeSC /= scEnvelopeAverage;

			float spectrumCompensationLow = fminf(RATIO_LIMIT, (lowEnvelope > ENVELOPE_MINIMUM) ? lowEnvelopeSC / lowEnvelope : 0.0f);
			float spectrumCompensationMid = fminf(RATIO_LIMIT, (midEnvelope > ENVELOPE_MINIMUM) ? midEnvelopeSC / midEnvelope : 0.0f);
			float spectrumCompensationHigh = fminf(RATIO_LIMIT, (highEnvelope > ENVELOPE_MINIMUM) ? highEnvelopeSC / highEnvelope : 0.0f);

			// Apply spectrum factor
			float spectrumCompensationLowAdjusted = (spectrumCompensationLow > 1.0f) ? 1.0f + ((spectrumCompensationLow - 1.0f) * spectrum) : 1.0f - ((1.0f - spectrumCompensationLow) * spectrum);
			float spectrumCompensationMidAdjusted = (spectrumCompensationMid > 1.0f) ? 1.0f + ((spectrumCompensationMid - 1.0f) * spectrum) : 1.0f - ((1.0f - spectrumCompensationMid) * spectrum);
			float spectrumCompensationHighAdjusted = (spectrumCompensationHigh > 1.0f) ? 1.0f + ((spectrumCompensationHigh - 1.0f) * spectrum) : 1.0f - ((1.0f - spectrumCompensationHigh) * spectrum);
			
			// Apply gain adjustment
			spectrumCompensationLowAdjusted *= gainRatioAdjusted;
			spectrumCompensationMidAdjusted *= gainRatioAdjusted;
			spectrumCompensationHighAdjusted *= gainRatioAdjusted;

			// Output
			const float out = lowAllPass * spectrumCompensationLowAdjusted + mid * spectrumCompensationMidAdjusted + high * spectrumCompensationHighAdjusted;
			// TO DO: Investigate volume spikes. EDIT: Should be better now
			mainChannelBuffer[sample] = fmaxf(-1.0f, fminf(volume * ((1.0f - mix) * in + mix * out), 1.0f));
		}
	}
}

//==============================================================================
bool EnvelopeClonerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EnvelopeClonerAudioProcessor::createEditor()
{
    return new EnvelopeClonerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void EnvelopeClonerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void EnvelopeClonerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout EnvelopeClonerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(0.1f, 10.0f, 0.1f, 0.7f), 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.7f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(-12.0f, 12.0f, 0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnvelopeClonerAudioProcessor();
}
