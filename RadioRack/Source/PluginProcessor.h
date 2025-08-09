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

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

#include "../../../zazzVSTPlugins/Shared/Filters/SmallSpeakerSimulation.h"
#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"
#include "../../../zazzVSTPlugins/Shared/Reverbs/RoomEarlyReflection.h"

//==============================================================================
class RadioRackAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    RadioRackAudioProcessor();
    ~RadioRackAudioProcessor() override;

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

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	SmallSpeakerSimulation m_spekaerSimulation[N_CHANNELS];
	RadioRackEarlyReflections m_earlyReflections[N_CHANNELS];
	ForthOrderHighPassFilter m_inputHighPass[N_CHANNELS];
	ForthOrderLowPassFilter m_inputLowPass[N_CHANNELS];
	
	BranchingEnvelopeFollowerUnsafe<float> m_gateEnvelope[N_CHANNELS];
	BranchingEnvelopeFollowerUnsafe<float> m_compressorEnvelope[N_CHANNELS];

	std::atomic<float>* m_thresholdParameter = nullptr;
	std::atomic<float>* m_compressionParameter = nullptr;
	std::atomic<float>* m_gainParameter = nullptr;
	std::atomic<float>* m_splitParameter = nullptr;
	std::atomic<float>* m_speakerTypeParameter = nullptr;
	std::atomic<float>* m_speakerResonanceParameter = nullptr;
	std::atomic<float>* m_speakerSizeParameter = nullptr;
	std::atomic<float>* m_volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadioRackAudioProcessor)
};
