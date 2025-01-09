/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================

const std::string NoiseEnhancerAudioProcessor::paramsNames[] = {	"AmpAttack",	"AmpDecay",		"AmpSustain",	"AmpSustainLevel",	"AmpRelease",
																	"FreqAttack0",	"FreqAttack1",	"FreqDecay",	"FreqSustain",		"FreqRelease",
																	"Frequency",	"Threshold",	"Amount",		"Volume" };
const std::string NoiseEnhancerAudioProcessor::labelNames[] = {		"Attack",		"Decay",		"Sustain",		"Sus. Level",	"Release",
																	"Attack 0",		"Attack 1",		"Decay",		"Susutain",		"Release",
																	"Frequency",	"Threshold",	"Amount",		"Volume" };
const std::string NoiseEnhancerAudioProcessor::paramsUnitNames[] = {	" ms",		" ms",			" ms",		" dB",			" ms",
																		" Hz",		" Hz",			" Hz",		" Hz",			" Hz",
																		" Hz",		" dB",			" %",		" dB" };

//==============================================================================
NoiseEnhancerAudioProcessor::NoiseEnhancerAudioProcessor()
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
	attackParameter			= apvts.getRawParameterValue(paramsNames[0]);
	decayParameter			= apvts.getRawParameterValue(paramsNames[1]);
	sustainParameter		= apvts.getRawParameterValue(paramsNames[2]);
	sustainLevelParameter   = apvts.getRawParameterValue(paramsNames[3]);
	releaseParameter		= apvts.getRawParameterValue(paramsNames[4]);

	freqA0Parameter			= apvts.getRawParameterValue(paramsNames[5]);
	freqA1Parameter			= apvts.getRawParameterValue(paramsNames[6]);
	freqDParameter			= apvts.getRawParameterValue(paramsNames[7]);
	freqSParameter			= apvts.getRawParameterValue(paramsNames[8]);
	freqRParameter			= apvts.getRawParameterValue(paramsNames[9]);

	frequencyParameter		= apvts.getRawParameterValue(paramsNames[10]);
	thresholdParameter		= apvts.getRawParameterValue(paramsNames[11]);
	amountParameter			= apvts.getRawParameterValue(paramsNames[12]);
	volumeParameter			= apvts.getRawParameterValue(paramsNames[13]);

	triggerSoloParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("TriggerSolo"));
	noiseSoloParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("NoiseSolo"));
}

NoiseEnhancerAudioProcessor::~NoiseEnhancerAudioProcessor()
{
}

//==============================================================================
const juce::String NoiseEnhancerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoiseEnhancerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoiseEnhancerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoiseEnhancerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoiseEnhancerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoiseEnhancerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoiseEnhancerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoiseEnhancerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoiseEnhancerAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoiseEnhancerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoiseEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_amplitudeEnvelope[0].init(sr);
	m_amplitudeEnvelope[1].init(sr);

	m_frequencyEnvelope[0].init(sr);
	m_frequencyEnvelope[1].init(sr);

	m_filter[0].init(sr);
	m_filter[1].init(sr);

	m_scFilter[0].init(sr);
	m_scFilter[1].init(sr);
}

void NoiseEnhancerAudioProcessor::releaseResources()
{
	m_amplitudeEnvelope[0].release();
	m_amplitudeEnvelope[1].release();

	m_frequencyEnvelope[0].release();
	m_frequencyEnvelope[1].release();

	m_filter[0].release();
	m_filter[1].release();

	m_scFilter[0].release();
	m_scFilter[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoiseEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NoiseEnhancerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto attack = attackParameter->load();
	const auto decay = decayParameter->load();
	const auto sustain = sustainParameter->load();
	const auto sustainLeveldB = sustainLevelParameter->load();
	const auto release = releaseParameter->load();

	const auto freqA0 = freqA0Parameter->load();
	const auto freqA1 = freqA1Parameter->load();
	const auto freqD = freqDParameter->load();
	const auto freqS = freqSParameter->load();
	const auto freqR = freqRParameter->load();

	const auto frequency = frequencyParameter->load();
	const auto threshold = Math::dBToGain(thresholdParameter->load());
	const auto amount = Math::dBToGain(60.0f * (0.01f * amountParameter->load() - 1.0f));
	const auto gain = Math::dBToGain(volumeParameter->load());

	const auto triggerSolo = triggerSoloParameter->get();
	const auto noiseSolo = noiseSoloParameter->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	const Envelope::EnvelopeParams params(	attack,
											decay,
											sustain,
											release,
											freqA0,
											freqA1,
											freqD,
											freqS,
											freqR);

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		//References
		auto& amplitudeEnvelope = m_amplitudeEnvelope[channel];
		auto& frequencyEnvelope = m_frequencyEnvelope[channel];
		auto& random = m_random[channel];
		auto& filter = m_filter[channel];
		auto& scFilter = m_scFilter[channel];

		//Set params
		amplitudeEnvelope.set(attack, decay, sustain, release, sustainLeveldB);
		frequencyEnvelope.set(params);
		scFilter.setBandPassPeakGain(frequency, 0.707f);

		if (triggerSolo)
		{
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Prepare side chain for trigger evaluation
				const float sideChain = scFilter.processDF1(in);

				channelBuffer[sample] = sideChain;
			}
		}
		else
		{
			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				// Prepare side chain for trigger evaluation
				const float sideChain = scFilter.processDF1(in);

				if (amplitudeEnvelope.isFinnished())
				{
					if (sideChain > threshold || sideChain < -threshold)
					{
						amplitudeEnvelope.reset();
						frequencyEnvelope.reset();

						const float envelopeGain = amplitudeEnvelope.process();
						float envelopeFrequency = frequencyEnvelope.process();
						filter.setLowPass(envelopeFrequency, 0.707f);
						const float noise = filter.processDF1(2.0f * random.process() - 1.0f);

						if (noiseSolo)
						{
							channelBuffer[sample] = amount * envelopeGain * noise;
						}
						else
						{
							channelBuffer[sample] = in + amount * envelopeGain * noise;
						}
					}
					else
					{
						if (noiseSolo)
						{
							channelBuffer[sample] = 0.0f;
						}
					}
				}
				else
				{
					const float envelopeGain = amplitudeEnvelope.process();
					float envelopeFrequency = frequencyEnvelope.process();
					filter.setLowPass(envelopeFrequency, 0.707f);
					const float noise = filter.processDF1(2.0f * random.process() - 1.0f);

					if (noiseSolo)
					{
						channelBuffer[sample] = amount * envelopeGain * noise;
					}
					else
					{
						channelBuffer[sample] = in + amount * envelopeGain * noise;
					}
				}
			}
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool NoiseEnhancerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoiseEnhancerAudioProcessor::createEditor()
{
    return new NoiseEnhancerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void NoiseEnhancerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void NoiseEnhancerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout NoiseEnhancerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 1.0f, 100.0f, 1.0f, 0.7f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 1.0f, 100.0f, 1.0f, 0.7f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 1.0f, 200.0f, 1.0f, 0.7f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -24.0f, 0.0f, 1.0f, 1.7f), -6.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( 1.0f, 200.0f, 1.0f, 0.7f), 50.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( 100.0f, 20000.0f, 1.0f, 0.4f),  100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( 100.0f, 20000.0f, 1.0f, 0.4f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>( 100.0f, 20000.0f, 1.0f, 0.4f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( 100.0f, 20000.0f, 1.0f, 0.4f),  1000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>( 100.0f, 20000.0f, 1.0f, 0.4f),   100.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(40.0f, 10000.0f, 1.0f, 0.4f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>(-48.0f, 0.0f, 1.0f, 1.0f), -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[13], paramsNames[13], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("TriggerSolo", "TriggerSolo", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("NoiseSolo", "NoiseSolo", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseEnhancerAudioProcessor();
}
