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
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

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

		m_quantizationLevels = std::exp2f(bitDepth);
		m_quantizationStep = 1.0f / m_quantizationLevels;

		m_filter.set(frequency);
	}
	inline float processFloor(const float in)
	{
		//BitCrush
		const float quantizated = static_cast<float>(static_cast<int>(in * m_quantizationLevels)) * m_quantizationStep;

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
	inline float processRound(const float in)
	{
		//BitCrush
		float quantizated = std::roundf(in * m_quantizationLevels) * m_quantizationStep;
		quantizated = std::fabsf(quantizated) < 0.001f ? 0.0f : quantizated;

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
	ForthOrderLowPassFilter m_filter;
	float m_holdValue = 0.0f;	
	float m_bitDepth = 8.0f;
	float m_quantizationLevels = 128.0f;
	float m_quantizationStep = 0.0078125f;
	int m_samplesToHold = 0;
	int m_downSample = 1;
};

//==============================================================================
class BitCrusherSmooth
{
public:
	BitCrusherSmooth() = default;
	~BitCrusherSmooth() = default;

	inline void set(const float bitDepth, const int downSample, const float drive) noexcept
	{
		m_bitDepth = bitDepth;
		m_downSample = downSample;

		m_quantizationLevels = 2.0f * std::exp2f(bitDepth);

		m_roughness = Math::remap(powf(1.0f - drive, 0.2f), 0.0f, 1.0f, 50.0f, 1.5f);
	}
	inline float process(const float in) noexcept
	{
		//BitCrush
		const float quantizated = processQuantization(in);

		// Downsample
		m_samplesToHold--;
		if (m_samplesToHold <= 0)
		{
			m_samplesToHold = m_downSample;
			m_holdValue = quantizated;
		}

		return m_holdValue;
	}
	inline void release()
	{
		m_holdValue = 0.0f;
		m_bitDepth = 8.0f;
		m_quantizationLevels = 128.0f;
		m_samplesToHold = 0;
		m_downSample = 1;
	}

private:
	inline float processQuantization(float in) noexcept
	{
		// Scale input
		const float inq = m_quantizationLevels * in;

		// Rescale input to range 0 - 1
		float inmod = std::fmodf(inq, 1.0f);
		if (inmod < 0.0f)
		{
			inmod += 1.0f;  // ensure positive fractional part
		}

		// Quantize input
		const int infloor = static_cast<int>(std::floorf(inq));

		// Create alternating value (0 or 1) - FIXED
		const float t = static_cast<float>(infloor & 1);

		// Calculate exponent
		const float exponent = t * m_roughness + (1.0f - t) * (1.0f / m_roughness);

		// Apply power warp
		const float inpow = std::powf(inmod, exponent);

		// Rescale and shift back
		return (inpow + infloor) / m_quantizationLevels;
	}

	float m_holdValue = 0.0f;
	float m_bitDepth = 8.0f;
	float m_quantizationLevels = 128.0f;
	float m_roughness = 50.0f;
	int m_samplesToHold = 0;
	int m_downSample = 1;
};