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

#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"

class BitCrusher
{
public:
	BitCrusher() = default;
	~BitCrusher() = default;

	inline void init(const int sampleRate)
	{
		m_filter.init(sampleRate);
	}
	inline void set(const float bitDepth, const int downSample, const float frequency)
	{
		m_bitDepth = bitDepth;
		m_downSample = downSample;

		m_quantizationLevels = std::powf(2.0f, m_bitDepth);
		m_quantizationStep = 1.0f / m_quantizationLevels;

		m_filter.set(frequency);
	}
	inline float process(const float in)
	{
		//BitCrush
		const float quantizated = static_cast<int>(in * m_quantizationLevels) * m_quantizationStep;

		//Filter
		const float filtered = m_filter.process(quantizated);

		// Downsample
		m_samplesToHold--;
		if (m_samplesToHold <= 0)
		{
			m_samplesToHold = m_downSample;
			m_holdValue = filtered;
		}

		return m_holdValue;
	}
	inline void release()
	{
		m_filter.release();
		m_holdValue = 0.0f;
		m_bitDepth = 8.0f;
		m_quantizationLevels = 128.0f;
		m_quantizationStep = 0.0078125f;
		m_samplesToHold = 0;
		m_downSample = 1;
	}

private:
	SixthOrderLowPassFilter m_filter;
	float m_holdValue = 0.0f;	
	float m_bitDepth = 8.0f;
	float m_quantizationLevels = 128.0f;
	float m_quantizationStep = 0.0078125f;
	int m_samplesToHold = 0;
	int m_downSample = 1;
};