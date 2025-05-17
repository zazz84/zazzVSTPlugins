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

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Random.h"

//==============================================================================
struct EarlyReflectionsParams
{
	float predelay = 0.0f;		// ms
	float length = 0.0f;		// ms
	float decay = 0.0f;			// dB
	float diffusion = 0.0f;
	float damping = 0.0f;
	float width = 0.0f;

	bool operator==(const EarlyReflectionsParams& l) const
	{
		return	Math::almostEquals(predelay, l.predelay, 0.01f) &&
				Math::almostEquals(length, l.length, 0.01f) &&
				Math::almostEquals(decay, l.decay, 0.01f) &&
				Math::almostEquals(diffusion, l.diffusion, 0.01f) &&
				Math::almostEquals(damping, l.damping, 0.01f) &&
				Math::almostEquals(width, l.width, 0.01f);
	};
};

//==============================================================================
class EarlyReflections : public CircularBuffer
{
public:
	EarlyReflections() = default;
	~EarlyReflections() = default;

	static const int DELAY_LINE_COUNT_MAX = 20;
	static const int DELAY_LINE_LENGHT_MAX_MS = 160;

	inline void init(const int sampleRate, const int channel)
	{
		m_sampleRate = sampleRate;
		m_channel = channel;
		
		// Set delay line
		const int size = DELAY_LINE_LENGHT_MAX_MS * sampleRate;
		__super::init(size);

		// Set dissution all-pass filters
		m_allPassFilter[0].init((int)(0.0021f * (float)sampleRate));
		m_allPassFilter[1].init((int)(0.0032f * (float)sampleRate));

		// Set damping filters
		for (int i = 0; i < m_delayLineCount; i++)
		{
			m_dampingFilter[i].init(sampleRate);
		}
	};
	inline void set(EarlyReflectionsParams& params) noexcept
	{	
		if (m_params == params)
		{
			return;
		}
		
		m_params = params;
		
		// Setup
		LinearCongruentialRandom01 randomChannel1 = {};
		randomChannel1.set(4L);

		LinearCongruentialRandom01 randomCurrentChannel = {};
		randomCurrentChannel.set((long)(m_channel + 1) + 16L);

		m_delayLineCount = Math::remap(params.length, 0.0f, 100.0f, 0.5f * DELAY_LINE_COUNT_MAX, DELAY_LINE_COUNT_MAX);
		const float stepSize = params.length / m_delayLineCount;
		const float randomRange = 0.85f * stepSize;
		const float predelaySamples = 0.001f * params.predelay * (float)m_sampleRate;

		const float stepGain = params.decay / (float)m_delayLineCount;

		for (int i = 0; i < m_delayLineCount; i++)
		{
			// Set delay times
			const float offsetChannel1 = randomRange * (2.0f * randomChannel1.process() - 1.0f);
			const float offsetCurrentChannel = randomRange * (2.0f * randomCurrentChannel.process() - 1.0f);
			const float offset = (1.0f - params.width) * offsetChannel1 + params.width * offsetCurrentChannel;
			
			const float delayTime = (float)(i + 1) * stepSize + offset;
			const int delayTimeSamples = (int)(0.001f * delayTime * (float)m_sampleRate);
			
			m_delayTimeSamples[i] = predelaySamples + delayTimeSamples;

			// Set gains
			const float gain = juce::Decibels::decibelsToGain(i * stepGain);
			m_delayLineGain[i] = gain;

			// Set filters
			m_dampingFilter[i].setCoef(0.8f * params.damping + (0.2f * i / m_delayLineCount));
		}
	};
	inline float process(const float in) noexcept
	{
		// Handle diffusion
		const float diffused = m_allPassFilter[0].process(m_allPassFilter[1].process(in));	
		write(m_params.diffusion * diffused + (1.0f - m_params.diffusion) * in);

		// Read all delay tabs
		float out = 0.0f;
		for (int i = 0; i < m_delayLineCount; i++)
		{
			out += m_delayLineGain[i] * m_dampingFilter[i].process(readDelay(m_delayTimeSamples[i]));
		}

		return out;
	};

private:
	OnePoleLowPassFilter m_dampingFilter[DELAY_LINE_COUNT_MAX];
	int m_delayTimeSamples[DELAY_LINE_COUNT_MAX];
	float m_delayLineGain[DELAY_LINE_COUNT_MAX];
	AllPassFilter m_allPassFilter[2];
	
	EarlyReflectionsParams m_params = {};
	
	int m_sampleRate = 48000;
	int m_delayLineCount = 0;
	int m_channel = 0;
};