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

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/TubeEmulation.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

//==============================================================================
class TubePreampAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    TubePreampAudioProcessor();
    ~TubePreampAudioProcessor() override;

	static const int N_CHANNELS = 2;
	static const int N_STAGES = 8;
	static const std::string paramsNames[];
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];

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
	TubeEmulation m_tubeEmulation[N_CHANNELS][N_STAGES];
	BiquadFilter m_preFilter[N_STAGES];
	BiquadFilter m_postFilter[N_STAGES];
	BranchingEnvelopeFollower<float> m_envelopeFollower[N_STAGES];

	std::atomic<float>* m_driveParameter = nullptr;
	std::atomic<float>* m_volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubePreampAudioProcessor)
};
