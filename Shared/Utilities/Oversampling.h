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

#include <cstring>

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/AudioBuffer.h"

class Oversampling
{
public:
	Oversampling() {};

	inline void init(const int sampleRate, const int oversamplingRation, const int samples)
	{
		m_samples = samples;
		m_oversampleSamples = oversamplingRation * samples;
		m_oversamplingRatio = oversamplingRation;
		
		m_buffer.init(m_oversampleSamples);
		
		// Set downsample filter
		m_downsampleFilter.init(sampleRate * oversamplingRation);
		m_downsampleFilter.setLowPass(10000.0f, 1.0);
	}
	inline void oversample(float* inputBuffer)
	{
		for (int sample = 0; sample < m_samples; sample++)
		{
			m_buffer.m_buffer[sample * m_oversamplingRatio] = inputBuffer[sample];
			m_buffer.m_buffer[sample * m_oversamplingRatio + 1] = inputBuffer[sample];
		}
	};
	inline void downsample(float* outputBuffer)
	{
		for (int overSample = 0; overSample < m_oversampleSamples; overSample++)
		{
			m_buffer.m_buffer[overSample] = m_downsampleFilter.processDF1(m_buffer.m_buffer[overSample]);
		}

		for (int sample = 0; sample < m_samples; sample++)
		{
			outputBuffer[sample] = m_buffer.m_buffer[sample * m_oversamplingRatio];
		}
	};
	inline void release()
	{
		m_buffer.release();

		m_oversamplingRatio = 2;
		m_samples = 0;
		m_oversampleSamples = 0;

	};
	inline float* getOversampeBuffer()
	{
		return m_buffer.m_buffer;
	};
	inline int getOversampeBufferSize()
	{
		return m_oversampleSamples;
	};

private:
	AudioBuffer m_buffer;
	BiquadFilter m_downsampleFilter;
	
	int m_oversamplingRatio = 2;
	int m_samples = 0;								// Stores original buffer size
	int m_oversampleSamples = 0;
};