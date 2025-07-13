/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string EnvelopeClonerAudioProcessor::paramsNames[] =		{ "Dynamics", "Spectrum", "Attack", "Release", "Mix", "Volume" };
const std::string EnvelopeClonerAudioProcessor::labelNames[] =		{ "Dynamics", "Spectrum", "Attack", "Release", "Mix", "Volume" };
const std::string EnvelopeClonerAudioProcessor::paramsUnitNames[] = { "", "", " ms", " ms", "", " dB" };
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

	m_lowMidFilter[0].set(midCrossOverFrequency);
	m_lowMidFilter[1].set(midCrossOverFrequency);
	m_midHighFilter[0].set(highCrossOverFrequency);
	m_midHighFilter[1].set(highCrossOverFrequency);
	m_allPassFilter[0].set(highCrossOverFrequency);
	m_allPassFilter[1].set(highCrossOverFrequency);

	m_lowMidFilterSC[0].set(midCrossOverFrequency);
	m_lowMidFilterSC[1].set(midCrossOverFrequency);
	m_midHighFilterSC[0].set(highCrossOverFrequency);
	m_midHighFilterSC[1].set(highCrossOverFrequency);
	m_allPassFilterSC[0].set(highCrossOverFrequency);
	m_allPassFilterSC[1].set(highCrossOverFrequency);

	const int rmsSizeFull = static_cast<int>(0.010 * sampleRate);
	const int rmsSizeHigh = static_cast<int>(0.03 * sampleRate);
	const int rmsSizeMid = static_cast<int>(0.005 * sampleRate);
	const int rmsSizeLow = static_cast<int>(0.010 * sampleRate);

	m_rmsFullSpectrum[0].init(rmsSizeFull);
	m_rmsFullSpectrum[1].init(rmsSizeFull);
	m_rmsFullSpectrumSC[0].init(rmsSizeFull);
	m_rmsFullSpectrumSC[1].init(rmsSizeFull);

	m_rmsBands[0][0].init(rmsSizeLow);
	m_rmsBands[0][1].init(rmsSizeLow);
	m_rmsBands[0][2].init(rmsSizeMid);
	m_rmsBands[1][0].init(rmsSizeMid);
	m_rmsBands[1][1].init(rmsSizeHigh);
	m_rmsBands[1][2].init(rmsSizeHigh);

	m_rmsBandsSC[0][0].init(rmsSizeLow);
	m_rmsBandsSC[0][1].init(rmsSizeLow);
	m_rmsBandsSC[0][2].init(rmsSizeMid);
	m_rmsBandsSC[1][0].init(rmsSizeMid);
	m_rmsBandsSC[1][1].init(rmsSizeHigh);
	m_rmsBandsSC[1][2].init(rmsSizeHigh);

	// Envelope smoothers
	m_envelopeDetection[0][0].init(sr);
	m_envelopeDetection[0][1].init(sr);
	m_envelopeDetection[0][2].init(sr);
	m_envelopeDetection[1][0].init(sr);
	m_envelopeDetection[1][1].init(sr);
	m_envelopeDetection[1][2].init(sr);
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

		// Envelopes smoothers
		auto& envelopeDetectionLow = m_envelopeDetection[channel][0];
		auto& envelopeDetectionMid = m_envelopeDetection[channel][1];
		auto& envelopeDetectionHigh = m_envelopeDetection[channel][2];

		envelopeDetectionLow.set(attack, release);
		envelopeDetectionMid.set(attack, release);
		envelopeDetectionHigh.set(attack, release);

		// RMS
		auto& rmsFullSpectrum = m_rmsFullSpectrum[channel];
		auto& rmsFullSpectrumSC = m_rmsFullSpectrumSC[channel];

		auto& rmsLow = m_rmsBands[channel][0];
		auto& rmsMid = m_rmsBands[channel][1];
		auto& rmsHigh = m_rmsBands[channel][2];

		auto& rmsLowSC = m_rmsBandsSC[channel][0];
		auto& rmsMidSC = m_rmsBandsSC[channel][1];
		auto& rmsHighSC = m_rmsBandsSC[channel][2];

;
		// Buffers
		auto mainChannelBuffer = mainBuffer.getWritePointer(channel);
		auto sideChainChannelBuffer = sideChainBuffer.getWritePointer(channel);

		for (int sample = 0; sample < samples; ++sample)
		{
			// Input
			const float in = mainChannelBuffer[sample];
			const float inSC = sideChainChannelBuffer[sample];

			const float inRMS = rmsFullSpectrum.process(in);
			const float scRMS = rmsFullSpectrumSC.process(inSC);

			const float gainRatio = fminf(RATIO_LIMIT, (inRMS > ENVELOPE_MINIMUM) ? scRMS / inRMS : 0.0f);

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
			float lowEnvelope = rmsLow.process(lowAllPass);
			float midEnvelope = rmsMid.process(mid);
			float highEnvelope = rmsHigh.process(high);

			float lowEnvelopeSC = rmsLowSC.process(lowAllPassSC);
			float midEnvelopeSC = rmsMidSC.process(midSC);
			float highEnvelopeSC = rmsHighSC.process(highSC);

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
			const float lowGainSmooth = envelopeDetectionLow.process(spectrumCompensationLowAdjusted);
			const float midGainSmooth = envelopeDetectionMid.process(spectrumCompensationMidAdjusted);
			const float highGainSmooth = envelopeDetectionHigh.process(spectrumCompensationHighAdjusted);

			// Handle data for gain meters
			const float lowGainSmoothdB = juce::Decibels::gainToDecibels(lowGainSmooth);
			const float midGainSmoothdB = juce::Decibels::gainToDecibels(midGainSmooth);
			const float highGainSmoothdB = juce::Decibels::gainToDecibels(highGainSmooth);

			if (std::fabsf(lowGainSmoothdB) > std::fabsf(m_maxGainLow))
			{
				m_maxGainLow = lowGainSmoothdB;
			}

			if (std::fabsf(midGainSmoothdB) > std::fabsf(m_maxGainMid))
			{
				m_maxGainMid = midGainSmoothdB;
			}

			if (std::fabsf(highGainSmoothdB) > std::fabsf(m_maxGainHigh))
			{
				m_maxGainHigh = highGainSmoothdB;
			}

			const float out = lowAllPass * lowGainSmooth + mid * midGainSmooth + high * highGainSmooth;
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
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.7f), 0.1f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(1.0f, 300.0f, 0.1f, 0.7f), 50.0f));
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
