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

#include "../../../zazzVSTPlugins/Shared/Delays/CombFilter.h"
#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================
// Design based on Pirkle_Designing Audio Effect Plugins In C - Chapter 17.5
// Designed from 4 comb filters in parallel followed by 2 all pass filter in series
// NOTE: Putting AP filters before comb filters sounds exactly the same

class SchroederReverb
{
public:
	SchroederReverb() = default;
	~SchroederReverb() = default;
	
	// Choose the delays to have the 1 : 1.5 ratio.
	// Choose delay times that have no common factors or divisors.
	// Taken times from ~30.0 ms, converted to samples using 48.0kHz, found closest higher prime number and converted back to miliseconds
	static constexpr float COMP_FILTER_DELAY_TIME_MS[] = { 30.1458333333f, 34.6458333333f, 39.7291666667f, 45.8958333333f };
	//static constexpr float ALLPASS_FILTER_DELAY_TIME_MS[] = { 1.5208333333f, 4.1875f };
	static constexpr float ALLPASS_FILTER_DELAY_TIME_MS[] = { 3.15f, 5.79f };

	static const int COMB_FILTER_COUNT = 4;
	static const int ALLPASS_FILTER_COUNT = 2;

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		const float sampleRateMS = 0.001f * (float)sampleRate;

		// Early reflections
		// ER predelay fixed to max 10.0 ms
		// ER lenght fixed to max 60.0 ms
		// LR predelay fixed to max 80.0 ms
		m_earlyReflections.init((int)(sampleRateMS * 80.0f));

		m_ERfilter.init(sampleRate);
		m_LRfilter.init(sampleRate);

		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			const int size = (int)(COMP_FILTER_DELAY_TIME_MS[i] * sampleRateMS);
			m_combFilter[i].init(size);
		}

		for (int i = 0; i < ALLPASS_FILTER_COUNT; i++)
		{
			const int size = (int)(ALLPASS_FILTER_DELAY_TIME_MS[i] * sampleRateMS);
			m_allpassFilter[i].init(size);
			m_allpassFilter[i].setFeedback(0.68f);
		}
	};
	inline void set(const float earlyReflectionsPredelayMS, const float earlyReflectionsSize, const float earlyReflectionsDamping, const float earlyReflectionsWidth, const float earlyReflectionsGain,
		            const float lateReflectionsPredelayMS, const float lateReflectionsSize, const float lateReflectionsDamping, const float lateReflectionsWidth, const float lateReflectionsGain) noexcept
	{
		// Early reflections
		m_ERPredelaySamples = 2 + (int)((float)m_sampleRate * 0.001f * earlyReflectionsPredelayMS);
		m_ERSizeGain = juce::Decibels::decibelsToGain(Math::remap(earlyReflectionsSize, 0.0f, 1.0f, -36.0f, -6.0f));
		m_ERSizeSamples =(int)(Math::remap(earlyReflectionsSize, 0.0f, 1.0f, 0.010f, 0.030f) * (float)m_sampleRate);

		m_ERfilter.setCoef(0.9f * earlyReflectionsDamping);
		m_ERgain = earlyReflectionsGain;

		// Late reflections
		m_LRPredelaySamples = 2 + (int)((float)m_sampleRate * 0.001f * lateReflectionsPredelayMS);	
		m_LRfilter.setCoef(0.9f * lateReflectionsDamping);

		const float rt60 = Math::remap(lateReflectionsSize, 0.0f, 1.0f, 0.1f, 2.0f);
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			m_combFilter[i].setTime(rt60, (float)m_sampleRate);
		}

		m_LRgain = 0.5f * lateReflectionsGain;
	};
	inline float process(const float in) noexcept
	{
		/*m_earlyReflections.write(in);
		
		const float LRIn = m_LRfilter.process(m_earlyReflections.readDelay(m_LRPredelaySamples));
		float out = 0.0f;
		
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			out += m_combFilter[i].process(LRIn);
		}

		// Use allpass filters to generate early reflections
		const float ERIn = m_ERfilter.process(m_earlyReflections.readDelay(m_ERPredelaySamples) + m_ERSizeGain * m_earlyReflections.readDelay(m_ERSizeSamples));
		out = m_ERgain * ERIn + m_LRgain * out;

		for (int i = 0; i < ALLPASS_FILTER_COUNT; i++)
		{
			out = m_allpassFilter[i].process(out);
		}

		return out;*/

		float APOut = in;

		for (int i = 0; i < ALLPASS_FILTER_COUNT; i++)
		{
			APOut = m_allpassFilter[i].process(APOut);
		}
		
		float out = 0.0f;
		
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			out += m_combFilter[i].process(APOut);
		}

		return out;
	};
	inline void release()
	{
		m_earlyReflections.release();

		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			m_combFilter[i].release();
		}

		for (int i = 0; i < ALLPASS_FILTER_COUNT; i++)
		{
			m_combFilter[i].release();
		}
	};

private:
	CircularBuffer m_earlyReflections;
	CombFilter m_combFilter[COMB_FILTER_COUNT];
	AllPassFilter m_allpassFilter[ALLPASS_FILTER_COUNT];
	OnePoleLowPassFilter m_ERfilter;
	OnePoleLowPassFilter m_LRfilter;
	float m_ERgain = 1.0f;
	float m_LRgain = 1.0f;
	float m_ERSizeGain = 0.0f;
	int m_ERPredelaySamples = 2;
	int m_ERSizeSamples = 2;
	int m_LRPredelaySamples = 2;
	int m_sampleRate = 48000;
};