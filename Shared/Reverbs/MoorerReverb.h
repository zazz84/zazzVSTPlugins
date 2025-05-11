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

#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Delays/CombFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

// https://freeverb3-vst.sourceforge.io/doc/Moorer_Reverb.PDF

class MoorerReverb
{
public:
	MoorerReverb() = default;
	~MoorerReverb() = default;

	static const int EARLY_REFLECTION_COUNT = 17;
	/*static constexpr float EARLY_REFLECTION_TIME_MS[] =	 { 
															4.3083900227f,
															17.2108843537f,
															0.9977324263f,
															4.3083900227f,
															0.2040816327f,
															2.7891156463f,
															16.0090702948f,
															2.6984126984f,
															8.7074829932f,
															1.4965986395f,
															0.7936507937f,
															1.7006802721f,
															9.5011337868f,
															0.0907029478f,
															1.79138322f,
															1.4965986395f,
															1.201814059f,
															4.3990929705f
														};*/

	static constexpr float EARLY_REFLECTION_TIME_MS[] = {
															12.9251700681f,
															51.6326530611f,
															2.9931972789f,
															12.9251700681f,
															0.6122448981f,
															8.3673469389f,
															48.0272108844f,
															8.0952380952f,
															26.1224489796f,
															4.4897959185f,
															2.3809523811f,
															5.1020408163f,
															28.5034013604f,
															0.2721088434f,
															5.37414966f,
															4.4897959185f,
															3.605442177f,
															13.1972789115f
														};

	static constexpr float EARLY_REFLECTION_GAIN[] =	{
															0.841f,
															0.504f,
															0.490f,
															0.379f,
															0.380f,
															0.346f,
															0.289f,
															0.272f,
															0.193f,
															0.217f,
															0.181f,
															0.180f,
															0.181f,
															0.176f,
															0.142f,
															0.167f,
															0.134f,
														};

	static const int COMB_FILTER_COUNT = 6;	
	static constexpr float COMB_FILTER_DELAY_MS[] = { 39.8866213152f, 44.1950113379f, 47.9138321995f, 51.9954648526f, 55.9410430839f, 60.0226757370f };
	//static constexpr float COMB_FILTER_DELAY_MS[] = { 49.9791666667f, 55.9791666667f, 60.9791666667f, 67.8958333333f, 72.0208333333f, 77.8958333333f };
	
	//static constexpr float COMB_FILTER_G_1[] = { 0.83f, 0.87f, 0.91f, 0.94f, 0.96f, 1.00f };
	//static constexpr float COMB_FILTER_G_1[] = { 1.00f, 0.96f, 0.94f, 0.91f, 0.87f, 0.83f };

	static constexpr float ALLPASS_FILTER_DELAY_MS = 6.9614512472f;

	inline void init(const int sampleRate)
	{
		m_sampleRateMS = (float)sampleRate * 0.001f;
		
		// Early reflections
		// ER predelay fixed to max 10.0 ms
		// ER lenght fixed to max 60.0 ms
		// LR predelay fixed to max 80.0 ms
		m_earlyReflections.init((int)((float)sampleRate * 0.001f * 80.0f));

		// Late reflections
		m_LRfilter.init(sampleRate);
		
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			const int size = (int)(0.001f * COMB_FILTER_DELAY_MS[i] * (float)sampleRate);
			m_combFilter[i].init(size, sampleRate);
		}

		const int size = (int)(0.001f * ALLPASS_FILTER_DELAY_MS * (float)sampleRate);
		m_allPassFilter.init(size);
	}
	inline void set(const float earlyReflectionsPredelayMS, const float earlyReflectionsSize, const float earlyReflectionsDamping, const float earlyReflectionsWidth, const float earlyReflectionsGain,
					const float lateReflectionsPredelayMS, const float lateReflectionsSize, const float lateReflectionsDamping, const float lateReflectionsWidth, const float lateReflectionsGain) noexcept
	{
		// Early reflections
		m_ERgain = 0.22f * earlyReflectionsGain;
		m_ERPredelayMS = earlyReflectionsPredelayMS;
		m_earlyReflectionsFactor = 0.17f + earlyReflectionsSize;			// Add 0.17 to have maximum lenght of 60ms

		// Late reflections
		m_LRfilter.setCoef(0.9f * lateReflectionsDamping);
		
		const float LRgainCompensation = juce::Decibels::decibelsToGain(Math::remap(lateReflectionsSize, 0.0f, 0.8f, -5.5f, -9.0f)) * juce::Decibels::decibelsToGain(Math::remap(lateReflectionsSize, 0.8f, 1.0f, 0.0f, -4.0f));
		m_LRgain = lateReflectionsGain * LRgainCompensation;
		m_LRPredelaySamples = 4 + lateReflectionsPredelayMS * m_sampleRateMS;

		const float rt60 = Math::remap(lateReflectionsSize, 0.0f, 1.0f, 0.1f, 2.0f);
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			auto& combFilter = m_combFilter[i];
			m_combFilter[i].setTime(rt60, m_sampleRateMS * 1000.0f);
			combFilter.setDamping(0.5f * lateReflectionsDamping);
		}
	}
	inline float process(const float in)
	{
		// Early reflections
		m_earlyReflections.write(in);
		
		float EROut = 0.0f;
		
		for (int i = 0; i < EARLY_REFLECTION_COUNT; i++)
		{
			EROut += m_earlyReflections.readDelay((int)((m_ERPredelayMS + m_earlyReflectionsFactor * EARLY_REFLECTION_TIME_MS[i]) * m_sampleRateMS));
		}
		
		// Late reflections
		float LRIn = m_LRfilter.process(m_earlyReflections.readDelay(m_LRPredelaySamples));
		float LROut = 0.0f;

		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			LROut += m_combFilter[i].process(LRIn);
		}

		return  m_ERgain * EROut + m_LRgain * m_allPassFilter.process(LROut);
	}
	inline void release()
	{
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			m_combFilter[i].release();
		}

		m_allPassFilter.release();
	}

private:
	CircularBuffer m_earlyReflections;
	LowPassCombFilter m_combFilter[COMB_FILTER_COUNT];
	AllPassFilter m_allPassFilter;
	OnePoleLowPassFilter m_LRfilter;

	float m_sampleRateMS = 48.0f;
	float m_ERPredelayMS = 0.0f;
	float m_ERgain = 1.0f;
	float m_LRgain = 1.0f;

	float m_earlyReflectionsFactor = 1.17f;
	int m_LRPredelaySamples = 4;
};