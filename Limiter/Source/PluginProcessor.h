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

#pragma once

#include <JuceHeader.h>

#include <array>

#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter2.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter3.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/AdaptiveReleaseTime.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/InterSamplePeak.h"

//==============================================================================
class LimiterAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{

public:
	//==============================================================================
	LimiterAudioProcessor();
	~LimiterAudioProcessor() override;

	enum Type
	{
		None,
		Dirty,
		Agressive,
		Clean,
		COUNT
	};

	static const std::string paramsNames[];
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];

	static const int N_CHANNELS = 2;
	static const float MAXIMUM_ATTACK_TIME_MS;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	float getPeakReductiondB()
	{
		const float gainReductiondB = m_gainReductiondB;
		m_gainReductiondB = 0.0f;

		return gainReductiondB;
	}

	float getAdaptiveReleaseTimeMS()
	{
		return m_adaptiveReleaseTimeMS;
	}

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	std::array<Limiter, N_CHANNELS> m_dirtyLimiter;
	std::array<Limiter2, N_CHANNELS> m_agressiveLimiter;
	std::array<Limiter3, N_CHANNELS> m_cleanLimiter;
	std::array<CircularBuffer, N_CHANNELS> m_circularBuffer;
	std::array<InterSamplePeak, N_CHANNELS> m_interSamplePeak;
	std::array<AdaptiveReleaseTime, N_CHANNELS> m_adaptiveReleaseTime;

	float m_gainReductiondB = 0.0f;
	float m_adaptiveReleaseTimeMS = 0.0f;
	OnePoleLowPassFilter m_adaptiveReleaseTimeSmoother;

	std::atomic<float>* typeParameter = nullptr;
	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* m_ispButton;
	juce::AudioParameterBool* m_clipButton;
	juce::AudioParameterBool* m_adaptiveButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessor)
};
