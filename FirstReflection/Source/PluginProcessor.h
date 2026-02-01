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

#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"

//==============================================================================
class FirstReflectionAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    FirstReflectionAudioProcessor();
    ~FirstReflectionAudioProcessor() override;

	enum Parameters
    {
        ListenerHeight,
		EmitterHeight,
		EmitterDistance,
		ReflectionVolume,
		ReflectionLPCutoff,
		Volume,
        COUNT
    };

    static const int N_CHANNELS = 2;
	static const float MAXIMUM_HEIGHT;
	static const float MAXIMUM_DELAY_TIME;
	static const ModernRotarySlider::ParameterDescription m_parametersDescritpion[];

    //==============================================================================
	float reflectionTimeDelay(float sourceHeight, float listenerHeight, float horizontalDistance, float soundSpeed = 343.0f)
	{
		// Direct path distance
		const float directDistance = std::sqrtf(horizontalDistance * horizontalDistance +
			(sourceHeight - listenerHeight) * (sourceHeight - listenerHeight));

		// Reflected path distance
		const float reflectedDistance = std::sqrtf(horizontalDistance * horizontalDistance +
			(sourceHeight + listenerHeight) * (sourceHeight + listenerHeight));

		// Path difference
		const float pathDifference = reflectedDistance - directDistance;

		// Time delay in seconds
		const float timeDelaySeconds = pathDifference / soundSpeed;

		return timeDelaySeconds;
	}
	
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
	std::array<std::atomic<float>*, Parameters::COUNT> m_parameters;

	CircularBuffer m_delayLine[N_CHANNELS];
	OnePoleLowPassFilter m_lowPassFilter[N_CHANNELS];
	OnePoleLowPassFilter m_delaySamplesSmoother[N_CHANNELS];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstReflectionAudioProcessor)
};
