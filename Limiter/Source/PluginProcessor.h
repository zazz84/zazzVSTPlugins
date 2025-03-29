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

#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter2.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Limiter3.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

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

	static const std::string paramsNames[];
	static const std::string labelNames[];
	static const std::string paramsUnitNames[];

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
		const float gainReductiondB = -Math::gainTodB(m_gainMin);
		m_gainMin = 1.0f;

		return gainReductiondB;
	}

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	Limiter m_limiter1[2];
	Limiter m_limiter2[2];
	SlopeClipper m_clipper[2];

	Limiter2 m_logLimiter[2];
	SlopeClipper m_clipper2[2];

	Limiter3 m_advancedLimiter[2];

	float m_gainMin = 1.0f;

	std::atomic<float>* typeParameter = nullptr;
	std::atomic<float>* gainParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessor)
};
