/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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

//==============================================================================

const std::string LimiterAudioProcessor::paramsNames[] = { "Type", "Gain", "Release", "Threshold", "Volume" };
const std::string LimiterAudioProcessor::labelNames[] =  { "Type", "Input", "Release", "Threshold", "Output" };
const std::string LimiterAudioProcessor::paramsUnitNames[] = {"", " dB", " ms", " dB", " dB" };

const float LimiterAudioProcessor::MAXIMUM_ATTACK_TIME_MS = 1.33f;

//==============================================================================
LimiterAudioProcessor::LimiterAudioProcessor()
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
	typeParameter      = apvts.getRawParameterValue(paramsNames[0]);
	gainParameter      = apvts.getRawParameterValue(paramsNames[1]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[2]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);

    m_ispButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ISP"));
    m_clipButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Clip"));
    m_adaptiveButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("AR"));
}

LimiterAudioProcessor::~LimiterAudioProcessor()
{
}

//==============================================================================
const juce::String LimiterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LimiterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LimiterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LimiterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LimiterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LimiterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LimiterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LimiterAudioProcessor::getProgramName (int index)
{
    return {};
}

void LimiterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LimiterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	const int attackSize = (int)(MAXIMUM_ATTACK_TIME_MS * 0.001 * sampleRate);		// Maximum attack time
	
	setLatencySamples(attackSize + 1);

	for (unsigned int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_dirtyLimiter[channel].init(sr, attackSize);
		m_dirtyLimiter[channel].setAttackSize(attackSize);

		m_agressiveLimiter[channel].init(sr, attackSize);
		m_agressiveLimiter[channel].setAttackSize(attackSize);

		m_cleanLimiter[channel].init(sr, attackSize);
		m_cleanLimiter[channel].setAttackSize(attackSize);

        m_circularBuffer[channel].init(attackSize);

        m_adaptiveReleaseTime[channel].init(sr);
        m_adaptiveReleaseTime[channel].set(0.1f, 50.0f, 0.05f);
	}

    m_adaptiveReleaseTimeSmoother.init(sr);
    m_adaptiveReleaseTimeSmoother.init(10.0f);
}

void LimiterAudioProcessor::releaseResources()
{
	for (unsigned int channel = 0; channel < N_CHANNELS; channel++)
	{
		m_dirtyLimiter[channel].release();
		m_agressiveLimiter[channel].release();
		m_cleanLimiter[channel].release();
        m_circularBuffer[channel].release();
        m_interSamplePeak[channel].release();
        m_adaptiveReleaseTime[channel].release();
	}

    m_adaptiveReleaseTimeSmoother.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LimiterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void LimiterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto type			    = static_cast<Type>(typeParameter->load());
	const auto inputGain	    = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto release		    = releaseParameter->load();
	const auto threshold	    = juce::Decibels::decibelsToGain(thresholdParameter->load());
	const auto outputGain	    = juce::Decibels::decibelsToGain(volumeParameter->load());
    const bool interSamplePeak  = m_ispButton->get();
    const bool clip             = m_clipButton->get();
    const bool adaptive         = m_adaptiveButton->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	
	buffer.applyGain(inputGain);

    //Get input peak
    float inPeak = 0.0f;

	// Process limiter
    auto processChannel = [&](auto& limiter, int channel, float* channelBuffer)
        {
            auto& circularBuffer = m_circularBuffer[channel];
            auto& isp = m_interSamplePeak[channel];
            auto& adaptiveReleaseTime = m_adaptiveReleaseTime[channel];
                      
            limiter.setReleaseTime(release);
            limiter.setThreshold(threshold);

            adaptiveReleaseTime.setThreshold(threshold);

            //float adaptiveReleaseMax = 0.0f;

            for (int sample = 0; sample < samples; sample++)
            {
                const auto in = channelBuffer[sample]; 
                const auto inDelayed = circularBuffer.read();
                circularBuffer.write(in);

                // Get peak for detection
                const auto peak = interSamplePeak ? isp.process(in) : isp.get(in);

                // Get input peak for GR meter
                inPeak = std::fmaxf(inPeak, std::fabsf(inDelayed));

                if (adaptive)
                {
                    const float adaptiveRelease = adaptiveReleaseTime.process(peak);
                    limiter.setReleaseTime(adaptiveRelease);

                    //adaptiveReleaseMax = std::fmaxf(adaptiveRelease, adaptiveReleaseMax);
                    m_adaptiveReleaseTimeMS = m_adaptiveReleaseTimeSmoother.process(adaptiveRelease);
                }

                channelBuffer[sample] = limiter.process(peak, inDelayed);
            }

            /*if (adaptive)
            {
                if (channel == 0)
                {
                    m_adaptiveReleaseTimeMS = adaptiveReleaseMax;
                }
                else
                {
                    m_adaptiveReleaseTimeMS = std::fmaxf(m_adaptiveReleaseTimeMS, adaptiveReleaseMax);
                }
            }*/
        };

    for (int channel = 0; channel < channels; channel++)
    {
        float* channelBuffer = buffer.getWritePointer(channel);

        switch (type)
        {
        case Type::Dirty:
            processChannel(m_dirtyLimiter[channel], channel, channelBuffer);
            break;

        case Type::Agressive:
            processChannel(m_agressiveLimiter[channel], channel, channelBuffer);
            break;

        default: // Clean
            processChannel(m_cleanLimiter[channel], channel, channelBuffer);
            break;
        }
    }

    // Add clipping
    if (clip)
    {
        for (int channel = 0; channel < channels; channel++)
        {
            Clippers::Params params;
            params.threshold = threshold;
            params.wet = 1.0f;
            params.buffer = buffer.getWritePointer(channel);
            params.samples = samples;

            Clippers::HardBlock(params);
        }
    }

    //Get output peak
    const float outPeak = buffer.getMagnitude(0, samples);

    // Update gain reduction
    const float gainReductiondB = Math::gainTodB(inPeak) - Math::gainTodB(outPeak);
    m_gainReductiondB = std::fmaxf(m_gainReductiondB, gainReductiondB);

    // Apply output gain
	buffer.applyGain(outputGain);
}

//==============================================================================
bool LimiterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LimiterAudioProcessor::createEditor()
{
    return new LimiterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void LimiterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void LimiterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout LimiterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(   1.0f,   3.0f,  1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.1f, 100.0f,  0.1f, 0.6f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( -30.0f,   0.0f,  0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>("ISP", "ISP", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("Clip", "Clip", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("AR", "AR", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LimiterAudioProcessor();
}
