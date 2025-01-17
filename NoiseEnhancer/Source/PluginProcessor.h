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

#pragma once

#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Oscillators/ADSR.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

//==============================================================================
class NoiseEnhancerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    NoiseEnhancerAudioProcessor();
    ~NoiseEnhancerAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];
    static const int N_CHANNELS = 2;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
	float getRMS() const
	{
		return m_peakLR;
	}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	std::array<BiquadFilter, N_CHANNELS> m_filter;
	std::array<BiquadFilter, N_CHANNELS> m_scFilter;
	std::array<AmplitudeEnvelope, N_CHANNELS> m_amplitudeEnvelope;
	std::array<Envelope, N_CHANNELS> m_frequencyEnvelope;
	std::array<LinearCongruentialRandom01, N_CHANNELS> m_random;

	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* decayParameter = nullptr;
	std::atomic<float>* sustainParameter = nullptr;
	std::atomic<float>* sustainLevelParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;

	std::atomic<float>* freqA0Parameter = nullptr;
	std::atomic<float>* freqA1Parameter = nullptr;
	std::atomic<float>* freqDParameter = nullptr;
	std::atomic<float>* freqSParameter = nullptr;
	std::atomic<float>* freqRParameter = nullptr;

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* amountParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	float m_peakLR = 0.0f;

	juce::AudioParameterBool* triggerSoloParameter = nullptr;
	juce::AudioParameterBool* noiseSoloParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseEnhancerAudioProcessor)
};