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

class VelvetNoiseReverb
{
public:
	VelvetNoiseReverb() = default;
	~VelvetNoiseReverb() = default;

	static const int VNC_COUNT = 16;
	static const int VNC_MAX_SIZE = 64;

	struct VNC
	{
	public:
		VNC() = default;
		~VNC() = default;

		inline void release()
		{
			m_positiveSize = 0;
			m_negativeSize = 0;
		}

		//! Holds indexes to positive values in velvet noise IR for given region
		int m_positiveIdx[VNC_MAX_SIZE];
		//! Holds indexes to negative values in velvet noise IR for given region
		int m_negativeIdx[VNC_MAX_SIZE];
		int m_positiveSize = 0;
		int m_negativeSize = 0;
	};

	inline void init(const int sampleRate, const float lengthSeconds, const long densitySeed = 79L, const long signSeed = 264L)
	{
		m_sampleRate = sampleRate;
		
		const int size = (int)(lengthSeconds * (float)sampleRate);
		m_buffer.init(size);

		// Generate velvet noise IR
		VelvetNoiseGenerator velvetNoiseGenerator;
		velvetNoiseGenerator.setSeed(densitySeed, signSeed);
		velvetNoiseGenerator.set(0.01f);

		m_velvetNoiseIR = new int8_t[size];

		std::memset(m_velvetNoiseIR, 0, 20);

		for (int i = 21; i < size; i++)
		{			
			m_velvetNoiseIR[i] = static_cast<int8_t>(velvetNoiseGenerator.process11());
		}
	
		// Generate exponantialy decaying gains for VNC
		const float decayRate = log(1000.0f);

		m_segmentCount = 0;
		for (int i = 1; i <= VNC_COUNT; i++)
		{
			m_segmentCount += i;
		}

		const float segmentTimeSeconds = 1.0f / m_segmentCount;
		float time = 0.0f;

		for (int i = 0; i < VNC_COUNT; i++)
		{
			time += (float)i * segmentTimeSeconds;
			m_gains[i] = expf(-decayRate * time);
		}
	}
	inline void set(const float lengthSeconds)
	{
		// Devide inpulse response to segments with non-uniform lenght		
		const float size = lengthSeconds * (float)m_sampleRate;
		const int segmentSize = (int)(size / (float)m_segmentCount);

		int segmentIdx[VNC_COUNT + 1];
		segmentIdx[0] = 0;

		for (int i = 1; i < VNC_COUNT; i++)
		{
			segmentIdx[i] = segmentIdx[i - 1] + i * segmentSize;
		}

		segmentIdx[VNC_COUNT] = size - 1;

		// Get indexes
		for (int i = 0; i < VNC_COUNT; i++)
		{
			auto& vnc = m_VNC[i];

			int positiveIndex = 0;
			int negativeIndex = 0;

			for (int j = segmentIdx[i]; j < segmentIdx[i + 1]; j++)
			{
				const int8_t value = m_velvetNoiseIR[j];

				if (value > 0)
				{
					if (positiveIndex < VNC_MAX_SIZE)
					{
						vnc.m_positiveIdx[positiveIndex] = j;
						positiveIndex++;
					}
				}
				else if (value < 0)
				{
					if (negativeIndex < VNC_MAX_SIZE)
					{
						vnc.m_negativeIdx[negativeIndex] = j;
						negativeIndex++;
					}
				}
			}

			vnc.m_positiveSize = positiveIndex;
			vnc.m_negativeSize = negativeIndex;
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

			for (int j = 0; j < vnc.m_positiveSize; j++)
			{
				positiveValue += m_buffer.readDelay(vnc.m_positiveIdx[j]);
			}

			for (int j = 0; j < vnc.m_negativeSize; j++)
			{
				negativeValue += m_buffer.readDelay(vnc.m_negativeIdx[j]);
			}

			out += m_gains[i] * (positiveValue - negativeValue);
		}

		return out;
	}
	inline void release()
	{
		delete[] m_velvetNoiseIR;
		m_velvetNoiseIR = nullptr;

		m_buffer.release();

		for (int i = 0; i < VNC_COUNT; i++)
		{
			m_VNC[i].release();
			m_gains[i] = 0.0f;
		}

		m_segmentCount = 0;
		m_sampleRate = 48000;
	}

private:
	CircularBuffer m_buffer;
	int8_t* m_velvetNoiseIR = nullptr;
	VNC m_VNC[VNC_COUNT] = {};
	float m_gains[VNC_COUNT];
	int m_segmentCount = 0;
	int m_sampleRate = 48000;
};

//==============================================================================
/*static const int8_t VELVET_NOISE1[16] = { 1, 0,-1, 0, 0, 0, 0, 0, 0, 1, 0, 0,-1, 0, 0, 0 };
static const int8_t VELVET_NOISE2[16] = { 0,-1, 0, 0,-1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 };
static const int8_t VELVET_NOISE3[16] = { 0, 0, 0,-1, 0, 1,-1, 0, 0, 0, 1, 0, 0, 0, 0, 0 };
static const int8_t VELVET_NOISE4[16] = { 0, 1, 0,-1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1 };
static const int8_t VELVET_NOISE5[16] = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1, 1, 0 };
static const int8_t VELVET_NOISE6[16] = { 0, 0, 0, 1, 0, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0, 1 };
static const int8_t VELVET_NOISE7[16] = { 0, 0, 0, 0, 0,-1, 1, 0,-1, 0, 1, 0, 0, 0, 0, 0 };
static const int8_t VELVET_NOISE8[16] = {-1, 0,-1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 };

static const int8_t VELVET_NOISE[128] = { 1, 0,-1, 0, 0, 1, 0,-1, 0, 1, 0, 0,-1, 0, 0, 0,
										  0,-1, 0, 0,-1, 0, 1, 1, 0, 0,-1, 0, 0, 0, 1, 0,
										  0, 0, 0,-1, 0, 1,-1, 1,-1, 0, 1, 0, 0, 0, 0, 0,
										  0, 1, 0,-1, 1, 0, 0, 0, 1,-1, 0, 0, 0, 0, 0,-1,
										  0, 0, 1, 0,-1, 0, 0, 0, 0, 1, 0, 0,-1,-1, 1, 0,
										  0, 0, 0,-1,-1, 1, 0, 0, 1,-1, 0, 0, 0, 0, 0, 1,
										  0, 0, 0, 0, 0,-1, 1, 0,-1, 0, 1, 0, 0, 1,-1, 0,
										 -1, 0,-1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1,-1, 0 };

//static const int8_t SEGMENT_LENGTH[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
static const int8_t SEGMENT_LENGTH[8] = { 8, 7, 6, 8, 4, 3, 2, 1 };

class VelvetNoiseReverb2
{
public:
	VelvetNoiseReverb2() = default;
	~VelvetNoiseReverb2() = default;

	static const int VNC_COUNT = 8;
	static const int VNC_SEGMENT_COUNT = 36;

	inline void init(const int sampleRate, const float lenghtMS)
	{
		const int size = 0.001f * lenghtMS * (float)sampleRate;
		m_buffer.init(size);

		m_sampleRate = sampleRate;
	}
	inline void set(const float lenghtMS)
	{
		// Define segment length, non-uniform
		const int size = m_buffer.getSize();
		const int segmentSize = size / VNC_SEGMENT_COUNT;

		m_segmentIdx[0] = 0;

		for (int i = 1; i < VNC_COUNT; i++)
		{
			m_segmentIdx[i] = m_segmentIdx[i - 1] + (int)SEGMENT_LENGTH[i - 1] * segmentSize;
		}

		m_segmentIdx[VNC_COUNT] = size;

		// Get read indexes
		int positiveIndex = 0;
		int negativeIndes = 0;

		for (int i = 0; i < VNC_COUNT; i++)
		{
			const float segmentLenght = (float)(m_segmentIdx[i + 1] - m_segmentIdx[i]) / 16.0f;

			for (int j = 0; j < 16; j++)
			{
				const int noiseIdx = 16 * i + j;
				
				if (VELVET_NOISE[noiseIdx] > 0)
				{
					m_vncPositiveIndexes[positiveIndex] = m_segmentIdx[i] + segmentLenght * j;
					positiveIndex++;
				}
				else if (VELVET_NOISE[noiseIdx] < 0)
				{
					m_vncNegativeIndexes[negativeIndes] = m_segmentIdx[i] + segmentLenght * j;
					negativeIndes++;
				}
			}
		}

		// Generate exponantialy decaying gains fro VNC
		const float decayRate = log(1000.0f / (0.001f * lenghtMS));

		for (int i = 0; i < VNC_COUNT; i++)
		{
			const float time = 0.5f * (float)(m_segmentIdx[i] + m_segmentIdx[i + 1]) / (float)m_sampleRate;
			m_gains[i] = expf(-decayRate * time);
		}
	}
	inline float process(const float in)
	{
		m_buffer.write(in);

		float out = 0.0f;

		for (int i = 1; i < VNC_COUNT; i++)
		{
			float vncOut = 0.0f;

			vncOut += m_buffer.readDelay(m_vncPositiveIndexes[i * 3]);
			vncOut += m_buffer.readDelay(m_vncPositiveIndexes[i * 3 + 1]);
			vncOut += m_buffer.readDelay(m_vncPositiveIndexes[i * 3 + 2]);
			vncOut -= m_buffer.readDelay(m_vncNegativeIndexes[i * 3]);
			vncOut -= m_buffer.readDelay(m_vncNegativeIndexes[i * 3 + 1]);
			vncOut -= m_buffer.readDelay(m_vncNegativeIndexes[i * 3 + 3]);

			vncOut *= m_gains[i];

			out += vncOut;
		}
		return out;
	}
	inline void release()
	{
		m_buffer.release();
	}

private:
	CircularBuffer m_buffer;
	float m_gains[VNC_COUNT];
	int m_segmentIdx[VNC_COUNT + 1];
	int m_vncPositiveIndexes[VNC_COUNT * 3];
	int m_vncNegativeIndexes[VNC_COUNT * 3];

	int m_sampleRate = 48000;
};*/