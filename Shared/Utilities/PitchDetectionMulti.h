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

#include <vector>

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/*class PitchDetectionMulti
{
public:
	static const int FFT_ORDER = 14;
	static const int FFT_SIZE = 1 << FFT_ORDER;

	struct Peak
	{
		int index;
		float value;
	};

	struct Spectrum
	{
		float frequency;
		float gain;
	};
	
	PitchDetectionMulti() : m_forwardFFT(FFT_ORDER), m_window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
	{
	}
	~PitchDetectionMulti() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_sampleRate = sampleRate;

		// Set arrays to zero
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);
	};
	inline void set(const int frequencyCount)
	{
		m_spectrum.clear();
		m_spectrum.resize(frequencyCount);
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
	inline void reset()
	{
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);

		m_fifoIndex = 0;
	}
	inline void release() noexcept
	{
		reset();

		m_sampleRate = 48000;
	}
	inline std::vector<Spectrum>& getSpectrum()
	{
		return m_spectrum;
	}

private:
inline void _getFrequency(int minDistanceSemitones = 1) noexcept
{
	// Apply window
	m_window.multiplyWithWindowingTable(m_fftData, FFT_SIZE);

	// Perform FFT
	m_forwardFFT.performRealOnlyForwardTransform(m_fftData);

	constexpr int NUM_PEAKS = 8;
	std::vector<Peak> peaks;
	peaks.reserve(FFT_SIZE / 2);

	// Collect magnitudes
	for (int i = 0; i < FFT_SIZE / 2; i++)
	{
		const float real = m_fftData[2 * i];
		const float imag = m_fftData[2 * i + 1];

		const float mag = std::sqrt(real * real + imag * imag);
		if (mag > 0.000251f) // ignore noise
			peaks.push_back({ i, mag });
	}

	if (peaks.empty())
		return;

	// Sort by descending amplitude
	std::sort(peaks.begin(), peaks.end(),
		[](const Peak& a, const Peak& b) { return a.value > b.value; });

	const float bucketHz = static_cast<float>(m_sampleRate) / FFT_SIZE;
	m_spectrum.clear();
	std::vector<int> selectedBins;

	for (const auto& peak : peaks)
	{
		if ((int)m_spectrum.size() >= NUM_PEAKS)
			break;

		const int index = peak.index;
		const float freq = (index + 0.5f) * bucketHz;

		bool tooClose = false;
		if (minDistanceSemitones > 0)
		{
			for (int selIndex : selectedBins)
			{
				float selFreq = (selIndex + 0.5f) * bucketHz;
				float semitoneDiff = 12.0f * std::log2(freq / selFreq);
				if (std::abs(semitoneDiff) < minDistanceSemitones)
				{
					tooClose = true;
					break;
				}
			}
		}

		if (tooClose)
			continue;

		// Compute magnitude
		const float real = m_fftData[2 * index];
		const float imag = m_fftData[2 * index + 1];
		float mag = std::sqrt(real * real + imag * imag);
		float gain = (2.0f * mag) / FFT_SIZE;

		m_spectrum.push_back({ freq, gain });
		selectedBins.push_back(index);
	}
}

	juce::dsp::FFT m_forwardFFT;
	juce::dsp::WindowingFunction<float> m_window;

	float m_fifo[FFT_SIZE];
	float m_fftData[2 * FFT_SIZE];

	std::vector<Spectrum> m_spectrum{ 0.0f };

	int m_fifoIndex = 0;
	int m_bucketIndexMin = 0;
	int m_bucketIndexMax = FFT_SIZE / 2;
	int m_sampleRate = 48000;
};*/

class PitchDetectionMulti
{
public:
	static const int FFT_ORDER = 12;
	static const int FFT_SIZE = 1 << FFT_ORDER;

	struct Peak
	{
		int index;
		float value;
	};

	struct Spectrum
	{
		float frequency;
		float gain;
	};

	PitchDetectionMulti()
		: m_forwardFFT(FFT_ORDER),
		m_window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
	{
		reset();
	}

	~PitchDetectionMulti() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_sampleRate = sampleRate;
		reset();
	}

	inline void set(const int frequencyCount)
	{
		m_spectrum.clear();
		m_spectrum.resize(frequencyCount);
	}

	// Feed samples continuously
	inline void process(const float sample) noexcept
	{
		m_fifo[m_fifoIndex++] = sample;

		if (m_fifoIndex == FFT_SIZE)
		{
			// Copy input to FFT buffer
			juce::zeromem(m_fftData, sizeof(m_fftData));
			memcpy(m_fftData, m_fifo, sizeof(m_fifo));

			// Apply window and perform FFT
			m_window.multiplyWithWindowingTable(m_fftData, FFT_SIZE);
			m_forwardFFT.performRealOnlyForwardTransform(m_fftData);

			// Accumulate magnitudes for averaging
			for (int i = 0; i < FFT_SIZE / 2; ++i)
			{
				const float real = m_fftData[2 * i];
				const float imag = m_fftData[2 * i + 1];
				const float mag = std::sqrt(real * real + imag * imag);

				m_accumMagnitude[i] += mag;
				m_accumCount[i]++;
			}

			m_fifoIndex = 0;
		}
	}

	inline void reset() noexcept
	{
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);
		std::fill(std::begin(m_accumMagnitude), std::end(m_accumMagnitude), 0.0f);
		std::fill(std::begin(m_accumCount), std::end(m_accumCount), 0);

		m_fifoIndex = 0;
	}

	inline void release() noexcept
	{
		reset();
		m_sampleRate = 48000;
	}

	// Compute spectrum when requested
	inline const std::vector<Spectrum>& getSpectrum(int numPeaks = 8, int minDistanceSemitones = 1)
	{
		_computeSpectrum(numPeaks, minDistanceSemitones);

		// Ensure the vector has exactly numPeaks elements
		while ((int)m_spectrum.size() < numPeaks)
			m_spectrum.push_back({ 20.0f, 0.0f });

		return m_spectrum;
	}

private:

	inline void _computeSpectrum(int numPeaks, int minDistanceSemitones) noexcept
	{
		// Collect averaged peaks
		std::vector<Peak> peaks;
		peaks.reserve(FFT_SIZE / 2);

		for (int i = 0; i < FFT_SIZE / 2; ++i)
		{
			if (m_accumCount[i] == 0) continue;

			float avgMag = m_accumMagnitude[i] / m_accumCount[i];
			if (avgMag > 0.000251f)
				peaks.push_back({ i, avgMag });
		}

		if (peaks.empty())
		{
			m_spectrum.clear();
			return;
		}

		// Sort by descending magnitude
		std::sort(peaks.begin(), peaks.end(),
			[](const Peak& a, const Peak& b) { return a.value > b.value; });

		const float bucketHz = static_cast<float>(m_sampleRate) / FFT_SIZE;
		m_spectrum.clear();
		std::vector<int> selectedBins;

		for (const auto& peak : peaks)
		{			
			const int index = peak.index;
			const float freq = (index + 0.5f) * bucketHz;
			
			// Limit low frequency bins
			if (freq < 20.0f)
			{
				continue;
			}
			else if (freq > 16000.0f)
			{
				continue;
			}
			
			if ((int)m_spectrum.size() >= numPeaks)
				break;	

			bool tooClose = false;
			if (minDistanceSemitones > 0)
			{
				for (int selIndex : selectedBins)
				{
					float selFreq = (selIndex + 0.5f) * bucketHz;
					float semitoneDiff = 12.0f * std::log2(freq / selFreq);
					if (std::abs(semitoneDiff) < minDistanceSemitones)
					{
						tooClose = true;
						break;
					}
				}
			}

			if (tooClose) continue;

			// Use averaged magnitude
			float avgMag = m_accumMagnitude[index] / m_accumCount[index];
			float gain = (2.0f * avgMag) / FFT_SIZE;

			m_spectrum.push_back({ freq, gain });
			selectedBins.push_back(index);
		}
	}

	juce::dsp::FFT m_forwardFFT;
	juce::dsp::WindowingFunction<float> m_window;

	float m_fifo[FFT_SIZE]{};
	float m_fftData[2 * FFT_SIZE]{};

	float m_accumMagnitude[FFT_SIZE / 2]{};
	int   m_accumCount[FFT_SIZE / 2]{};

	std::vector<Spectrum> m_spectrum;

	int m_fifoIndex = 0;
	int m_sampleRate = 48000;
};