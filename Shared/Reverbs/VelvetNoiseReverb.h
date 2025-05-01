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

// https://www.researchgate.net/publication/316802594_Late_Reverberation_Synthesis_Using_Filtered_Velvet_Noise

#pragma once

#include <vector>

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/NoiseGenerator.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"

class VelvetNoiseReverb
{
public:
	VelvetNoiseReverb() = default;
	~VelvetNoiseReverb() = default;

	static constexpr int VNC_COUNT = 16;
	static constexpr int VNC_SEGMENT_COUNT = 136; // = 1 + 2 + 3 + ... + 16
	static constexpr int VNC_MAX_SIZE = 64;
	static constexpr float kOneOverSegmentCount = 1.0f / static_cast<float>(VNC_SEGMENT_COUNT);
	static constexpr float kLog1000 = 6.90775527898f; // log(1000)

	struct VNC
	{
	public:
		VNC() = default;
		~VNC() = default;

		//! Holds indexes to positive values in velvet noise IR for given region
		int m_positiveIdx[VNC_MAX_SIZE];
		//! Holds indexes to negative values in velvet noise IR for given region
		int m_negativeIdx[VNC_MAX_SIZE];
	};

	inline void init(const int sampleRate, const float lengthSeconds)
	{
		m_sampleRate = sampleRate;
		
		const int size = (int)(lengthSeconds * (float)sampleRate);
		m_buffer.init(size);
	
		// Filters
		for (int i = 0; i < VNC_COUNT; i++)
		{
			m_highPass[i].init(sampleRate);
			m_lowPass[i].init(sampleRate);
		}
	}
	inline void set(const float lengthSeconds, const float preDelaySeconds, const float decayFactor, const float density, const long seed, const float low, const float high)
	{
		m_VNCSize = 4 + (int)((float)(VNC_MAX_SIZE - 4) * density);
		
		// Generate exponantialy decaying gains
		const float decayRate = kLog1000 * decayFactor;
		float time = 0.0f;

		for (int i = 0; i < VNC_COUNT; i++)
		{
			time += (float)i * kOneOverSegmentCount;
			m_gains[i] = expf(-decayRate * time);
		}

		// Add lead in to the IR
		m_gains[0] = 0.5f * m_gains[0];

		// Devide inpulse response to segments with non-uniform lenght		
		const float size = lengthSeconds * (float)m_sampleRate;
		const int segmentSize = (int)(size / (float)VNC_SEGMENT_COUNT);
		const int predelaySize = (int)(preDelaySeconds * (float)m_sampleRate);

		int segmentIdx[VNC_COUNT + 1];
		segmentIdx[0] = predelaySize + 1;

		for (int i = 1; i < VNC_COUNT; i++)
		{
			segmentIdx[i] = segmentIdx[i - 1] + i * segmentSize;
		}

		segmentIdx[VNC_COUNT] = (int)size + predelaySize - 1;

		// Generate velvet noise indexes
		LinearCongruentialNoiseGenerator noiseGenerator;
		noiseGenerator.setSeed(seed);
		
		// Rest of the segments
		for (int i = 0; i < VNC_COUNT; i++)
		{
			auto& vnc = m_VNC[i];

			const int currSegmentOffset = segmentIdx[i];
			const int currSegmentSize = segmentIdx[i + 1] - currSegmentOffset;
			
			for (int j = 0; j < m_VNCSize; j++)
			{
				vnc.m_positiveIdx[j] = currSegmentOffset + (int)(noiseGenerator.process() * currSegmentSize);
				vnc.m_negativeIdx[j] = currSegmentOffset + (int)(noiseGenerator.process() * currSegmentSize);
			}
		}

		// Filters
		const float highPassQ = 0.5f + low * 0.5f;
		const float lowPassQ = 0.5f + high * 0.7f;

		const float highPassA = 20.0f + 100.0f * (1.0f - low);
		const float highPassB = 0.47f * (1.0f - low);

		const float lowPassA = 8000.0f + 8000.0f * high;
		const float lowPassB = 0.6f * high - 1.3f;

		for (int i = 0; i < VNC_COUNT; i++)
		{
			const float highFrequency = highPassA * powf(static_cast<float>(i + 1), highPassB);
			const float lowPassFrequency = lowPassA * powf(static_cast<float>(i + 1), lowPassB);
			m_highPass[i].setHighPass(highFrequency, highPassQ);
			m_lowPass[i].setLowPass(lowPassFrequency, lowPassQ);
		}
	}
	inline float process(const float in) noexcept
	{
		m_buffer.write(in);

		float out = 0.0f;

		for (int i = 0; i < VNC_COUNT; i++)
		{
			auto& vnc = m_VNC[i];
			
			float positiveValue = 0.0f;
			float negativeValue = 0.0f;

			for (int j = 0; j < m_VNCSize; j++)
			{
				positiveValue += m_buffer.readDelay(vnc.m_positiveIdx[j]);
				negativeValue += m_buffer.readDelay(vnc.m_negativeIdx[j]);
			}

			float temp = positiveValue - negativeValue;
			temp = m_lowPass[i].processDF1(temp);
			temp = m_highPass[i].processDF1(temp);

			out += m_gains[i] * temp;
		}

		return out;
	}
	inline void release()
	{
		m_buffer.release();

		for (int i = 0; i < VNC_COUNT; i++)
		{
			m_gains[i] = 0.0f;
			m_lowPass[i].release();
			m_highPass[i].release();
		}

		m_sampleRate = 48000;
		m_VNCSize = VNC_MAX_SIZE;
	}

private:
	CircularBuffer m_buffer;

	VNC m_VNC[VNC_COUNT] = {};
	BiquadFilter m_lowPass[VNC_COUNT] = {};
	BiquadFilter m_highPass[VNC_COUNT] = {};
	
	float m_gains[VNC_COUNT];
	int m_sampleRate = 48000;
	int m_VNCSize = VNC_MAX_SIZE;
};