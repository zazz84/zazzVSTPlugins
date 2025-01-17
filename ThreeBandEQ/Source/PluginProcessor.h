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

#include "../../../zazzVSTPlugins/Shared/Filters/FirstOrderAllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/LinkwitzRileyFilter.h"

//==============================================================================
class ThreeBandEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    ThreeBandEQAudioProcessor();
    ~ThreeBandEQAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const std::string labelNames[];

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

	inline float FrequencyToMel(float frequency)
	{
		return 2595.0f * log10f(1.0f + frequency / 700.0f);
	}
	inline float MelToFrequency(float mel)
	{
		return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
	}
	inline float NoteToFrequency(float note)
	{
		const float f0 = 440.0f;
		const float a = powf(2, 1.0f / 12.0f);

		const float fn = f0 * powf(a, note);
		
		return fn;
	}

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================

	std::atomic<float>* gainLowParameter = nullptr;
	std::atomic<float>* frequencyLowMidParameter = nullptr;
	std::atomic<float>* gainMidParameter = nullptr;
	std::atomic<float>* frequencyMidHighParameter = nullptr;
	std::atomic<float>* gainHighParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	LinkwitzRileyFilter m_lowMidFilter[2] = {};
	LinkwitzRileyFilter m_midHighFilter[2] = {};
	FirstOrderAllPassFilter m_allPassFilter[2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessor)
};
