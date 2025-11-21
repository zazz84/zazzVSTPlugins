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
#include <vector>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Utilities/PitchDetectionMulti.h"
#include "../../../zazzVSTPlugins/Shared/Oscillators/SinSawOscillator.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/NoiseGenerator.h"

//==============================================================================
class ResynthesizerAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    ResynthesizerAudioProcessor();
    ~ResynthesizerAudioProcessor() override;

	enum Parameters
    {
        Frequency1,
        Frequency2,
        Frequency3,
        Frequency4,
        Frequency5,
        Frequency6,
        Frequency7,
        Frequency8,
		Volume1,
		Volume2,
		Volume3,
		Volume4,
		Volume5,
		Volume6,
		Volume7,
		Volume8,
		Volume,
		MinimumStep,
		Shape,
		Factor,
		Style,
        COUNT
    };

    static const std::string paramsNames[];
    static const std::string labelNames[];
	static const std::string paramsUnitNames[];
    static const int N_CHANNELS = 2;
    static const int N_OSCILATORS = 8;

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
	void setParameterRaw(const juce::String& paramID, float rawValue);

	//==============================================================================
	PitchDetectionMulti m_pitchDetection;

	std::array<std::atomic<float>*, Parameters::COUNT> m_parameters;
	std::array<SinSawOscillator, N_OSCILATORS> m_oscilators;
	std::array<PinkNoiseGenerator, N_OSCILATORS> m_noiseGenerators;
	std::array<BiquadFilter, N_OSCILATORS> m_noiseFilters;
	std::vector<PitchDetectionMulti::Spectrum> m_spectrum;
	juce::AudioParameterBool* m_learnButton;
	bool m_learnButtonLast{ false };

	OnePoleLowPassFilter m_smoother;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResynthesizerAudioProcessor)
};
