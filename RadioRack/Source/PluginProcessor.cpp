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
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

//==============================================================================

const std::string RadioRackAudioProcessor::paramsNames[] =		{ "Threshold", "Compression", "Drive", "Split", "Type", "Resonance", "Size", "Volume" };
const std::string RadioRackAudioProcessor::labelNames[] =		{ "Threshold", "Compression", "Drive", "Split", "Type", "Resonance", "Size", "Volume" };
const std::string RadioRackAudioProcessor::paramsUnitNames[] =	{ " dB",       " %",          " dB",  " %",    "",     " %",        " %",   " dB" };

//==============================================================================
RadioRackAudioProcessor::RadioRackAudioProcessor()
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
	m_thresholdParameter		= apvts.getRawParameterValue(paramsNames[0]);
	m_compressionParameter		= apvts.getRawParameterValue(paramsNames[1]);
	m_gainParameter				= apvts.getRawParameterValue(paramsNames[2]);
	m_splitParameter			= apvts.getRawParameterValue(paramsNames[3]);
	m_speakerTypeParameter		= apvts.getRawParameterValue(paramsNames[4]);
	m_speakerResonanceParameter = apvts.getRawParameterValue(paramsNames[5]);
	m_speakerSizeParameter		= apvts.getRawParameterValue(paramsNames[6]);
	m_volumeParameter			= apvts.getRawParameterValue(paramsNames[7]);
}

RadioRackAudioProcessor::~RadioRackAudioProcessor()
{
}

//==============================================================================
const juce::String RadioRackAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RadioRackAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RadioRackAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RadioRackAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RadioRackAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RadioRackAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RadioRackAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RadioRackAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RadioRackAudioProcessor::getProgramName (int index)
{
    return {};
}

void RadioRackAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RadioRackAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	// Resonance
	const int predelaySize = (int)(1.0 * 0.001 * sampleRate);
	const int reflectionsSize = (int)(11.0 * 0.001 * sampleRate);
	const int size = predelaySize + reflectionsSize;

	for (int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_gateEnvelope[channel].init(sr);
		m_gateEnvelope[channel].set(0.1f, 3.0f);
		
		m_compressorEnvelope[channel].init(sr);
		// Attack and realease is probably swapped
		m_compressorEnvelope[channel].set(0.1f, 80.0f);

		m_spekaerSimulation[channel].init(sr);

		m_inputHighPass[channel].init(sr);
		m_inputLowPass[channel].init(sr);

		m_inputHighPass[channel].set(300.0f);
		m_inputLowPass[channel].set(3600.0f);

		m_earlyReflections[channel].init(size);
		m_earlyReflections[channel].set(predelaySize, reflectionsSize, -10.0f);
	}
}

void RadioRackAudioProcessor::releaseResources()
{
	m_spekaerSimulation[0].release();
	m_spekaerSimulation[1].release();

	m_earlyReflections[0].release();
	m_earlyReflections[1].release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RadioRackAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RadioRackAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	
	// Get params
	const auto drive					= Math::dBToGain(m_gainParameter->load());
	const auto gateThresholddB			= m_thresholdParameter->load();
	const auto compressorThresholddB	= -36.0f;
	const auto gateThreshold			= Math::dBToGain(gateThresholddB);
	const auto compressorThreshold		= Math::dBToGain(compressorThresholddB);
	const auto splitOffset				= powf(0.01f * m_splitParameter->load(), 3.0f);
	const auto compressionWet			= 0.01f * m_compressionParameter->load();
	const auto compressionDry			= 1.0f - compressionWet;
	const auto splitThreshold			= Math::dBToGain(-75.0f);

	const auto type = (int)m_speakerTypeParameter->load();
	const auto tune = m_speakerSizeParameter->load();
	const auto resonanceWet = 0.01f * m_speakerResonanceParameter->load();
	const auto resonanceDry = 1.0f - resonanceWet;

	const auto gain = juce::Decibels::decibelsToGain(m_volumeParameter->load());

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto compressorGainCompensation = Math::dBToGain(24.0f) * compressionWet;			// Inlcudes compressor wetness
	const auto distortionGainCompensation = drive > 0.0f ? fmaxf(1.0f / drive, Math::dBToGain(-18.0f)) : 1.0f / drive;

	for (int channel = 0; channel < channels; channel++)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		auto& gateEnvelope = m_gateEnvelope[channel];
		auto& compressorEnvelope = m_compressorEnvelope[channel];

		auto& speakerSimulation = m_spekaerSimulation[channel];
		auto& earlyReflections = m_earlyReflections[channel];

		auto& inputLowPass = m_inputLowPass[channel];
		auto& inputHighPass = m_inputHighPass[channel];

		speakerSimulation.set(type, tune);

		for (int sample = 0; sample < samples; sample++)
		{
			// Read
			float out = channelBuffer[sample];
			
			// Input filter
			out = inputLowPass.process(inputHighPass.process(out));
			
			// ABS
			float outAbs = Math::fabsf(out);

			// Gate
			float gateGain = gateEnvelope.process((float)(outAbs > gateThreshold));

			// Split
			if (outAbs > splitThreshold)
			{
				if (out > 0.0f)
				{
					out += splitOffset;
					outAbs = out;
				}
				else
				{
					out -= splitOffset;
					outAbs += splitOffset;
				}
			}

			// Limiter/Compressor
			float overThresholddB = 0.0f;

			if (outAbs > compressorThreshold)
			{
				const float outdB = Math::gainTodBAprox(outAbs);
				overThresholddB = outdB - compressorThresholddB;
			}

			const float gainReductiondB = compressorEnvelope.process(overThresholddB);
			const float gainReductionGain = 1.0f / Math::dBToGainAprox(gainReductiondB);
			 
			// Reciprocal waveshaper
			out = drive * gateGain * (out * (gainReductionGain * compressorGainCompensation + compressionDry));
			out = out / (1.0f + Math::fabsf(out));
			out *= distortionGainCompensation;											// apply oposite gain as gain reduction

			// Early reflections
			out = resonanceDry * out + resonanceWet * earlyReflections.process(out);

			// EQ
			out = speakerSimulation.process(out);
		
			//Out
			channelBuffer[sample] = gain * out;
		}
	}
}

//==============================================================================
bool RadioRackAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RadioRackAudioProcessor::createEditor()
{
    return new RadioRackAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void RadioRackAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void RadioRackAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout RadioRackAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -80.0f,   0.0f,  1.0f, 1.0f), -48.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( -48.0f,  48.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   1.0f,   9.0f,  1.0f, 1.0f),  1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(   0.0f, 100.0f,  1.0f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(  50.0f, 200.0f,  1.0f, 1.0f),100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>( -48.0f,  48.0f,  0.1f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RadioRackAudioProcessor();
}
