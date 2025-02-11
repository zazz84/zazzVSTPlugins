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

#pragma

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class PitchDetection
{
public:
	static const int FFT_ORDER = 12;
	static const int FFT_SIZE = 1 << FFT_ORDER;
	
	PitchDetection() : m_forwardFFT(FFT_ORDER), m_window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
	{
	}
	~PitchDetection() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_sampleRate = sampleRate;

		// Set arrays to zero
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);
	};
	inline void set(const float frequencyMin, const float frequencyMax)
	{
		const int bucketFrequency = m_sampleRate / FFT_SIZE;
		
		m_bucketIndexMin = static_cast<int>(frequencyMin) / bucketFrequency;
		m_bucketIndexMax = static_cast<int>(frequencyMax) / bucketFrequency;
	}
	inline void process(const float sample) noexcept
	{
		m_fifo[m_fifoIndex] = sample;
		m_fifoIndex++;

		if (m_fifoIndex == FFT_SIZE)
		{
			juce::zeromem(m_fftData, sizeof(m_fftData));
			memcpy(m_fftData, m_fifo, sizeof(m_fifo));

			_getFrequency();

			m_fifoIndex = 0;
		}
	}
	inline void release() noexcept
	{
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);

		m_fifoIndex = 0;
		m_sampleRate = 48000;
	}
	inline float getFrequency()
	{
		return m_frequency;
	}

private:
	inline void _getFrequency() noexcept
	{
		// Apply windowing function
		m_window.multiplyWithWindowingTable(m_fftData, FFT_SIZE);

		// Perform FFT
		m_forwardFFT.performFrequencyOnlyForwardTransform(m_fftData);

		// Get maximum index
		int maxIndex = m_bucketIndexMin;
		float maxValue = m_fftData[m_bucketIndexMin];

		for (int i = m_bucketIndexMin; i < m_bucketIndexMax; i++)
		{
			const float value = m_fftData[i];
			if (value > maxValue)
			{
				maxValue = value;
				maxIndex = i;
			}
		}

		// Ignore, if amplitude is too low
		if (maxValue < Math::dBToGain(-72.0f))
		{
			return;
		}

		// Convert maximum inde to frequency
		const int bucketFrequency = m_sampleRate / FFT_SIZE;

		m_frequency = static_cast<float>(maxIndex * bucketFrequency + bucketFrequency / 2);

		// TODO: Interpolate between maximum and largest neighbour
	}

	juce::dsp::FFT m_forwardFFT;
	juce::dsp::WindowingFunction<float> m_window;

	float m_fifo[FFT_SIZE];
	float m_fftData[2 * FFT_SIZE];

	float m_frequency;

	int m_fifoIndex = 0;
	int m_bucketIndexMin = 0;
	int m_bucketIndexMax = FFT_SIZE / 2;
	int m_sampleRate = 48000;
};