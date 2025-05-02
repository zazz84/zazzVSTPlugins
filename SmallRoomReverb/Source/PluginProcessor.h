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

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Reverbs/RoomEarlyReflection.h"

//==============================================================================
class Allpass : public CircularBuffer
{
public:
	Allpass() = default;
	~Allpass() = default;

	float process(float in)
	{
		const float delayOut = read();
				
		write(in + 0.5f * delayOut);
		
		return 0.5f * (delayOut - in);
	};
};

//==============================================================================
class CombFilter : public CircularBuffer
{
public:
	CombFilter() = default;
	~CombFilter() = default;
	
	inline void setGain(float gain) { m_gain = gain; };
	inline void setDamping(float dampingFactor)
	{
		m_dampingFactor = dampingFactor;
	};
	inline float process(float in)
	{
		const float delayOut = read();

		m_filterLast = delayOut - m_dampingFactor * (delayOut - m_filterLast);

		write(in + m_filterLast * m_gain);

		return delayOut;
	};

private:
	float m_gain = 1.0f;
	float m_dampingFactor = 0.0f;
	float m_filterLast = 0.0f;
};

//==============================================================================
class SmallRoomReverbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    SmallRoomReverbAudioProcessor();
    ~SmallRoomReverbAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	
	static const int N_ALLPASSES = 6;
	static const int BUFFER_MINIMUM_SIZE = 2;
	static const int MAX_CHANNELS = 2;
	static const float ALLPASS_DELAY_TIMES_MS[];
	static const float ALLPASS_DELAY_WIDTH[3][6];

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
	RoomEarlyReflectionsSimple m_earlyReflections[MAX_CHANNELS];
	Allpass m_allpass[MAX_CHANNELS][N_ALLPASSES];

	LowPassBiquadFilter m_ERfilter[MAX_CHANNELS];
	LowPassBiquadFilter m_LRfilter[MAX_CHANNELS];

	std::atomic<float>* ERpredelayParameter = nullptr;
	std::atomic<float>* ERsizeParameter = nullptr;
	std::atomic<float>* ERdampingParameter = nullptr;
	std::atomic<float>* ERwidthParameter = nullptr;

	std::atomic<float>* LRpredelayParameter = nullptr;
	std::atomic<float>* LRsizeParameter = nullptr;
	std::atomic<float>* LRdampingParameter = nullptr;
	std::atomic<float>* LRwidthParameter = nullptr;

	std::atomic<float>* ERvolumeParameter = nullptr;
	std::atomic<float>* LRvolumeParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmallRoomReverbAudioProcessor)
};
