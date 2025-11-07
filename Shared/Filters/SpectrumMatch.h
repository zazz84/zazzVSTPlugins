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
#include <array>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

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

class SpectrumDetectionFFT
{
public:
	static const int FFT_ORDER = 10;
	static const int FFT_SIZE = 1 << FFT_ORDER;

	static const int BANDS_COUNT = 6;
	inline static int FILTER_FREQUENCY[BANDS_COUNT] = { 50, 120, 380, 1000, 3300, 10000 };
	inline static float GAINS_ADJUSTION[] =
	{
		0.223872f,  // -13 dB
		0.177828f,  // -15 dB
		0.630957f,  // -4 dB
		0.707946f,  // -3 dB
		1.584893f,  // 4 dB
		2.661000f   // 8.5 dB
	};

	SpectrumDetectionFFT() : m_forwardFFT(FFT_ORDER), m_window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann)
	{
	}
	~SpectrumDetectionFFT() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_sampleRate = sampleRate;

		// Set arrays to zero
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);

		// Set bucket indexes
		const int bucketFrequency = m_sampleRate / FFT_SIZE;
		m_bucketIndex[0] = 0;

		for (size_t i = 0; i < BANDS_COUNT + 1; i++)
		{
			m_bucketIndex[i + 1] = FILTER_FREQUENCY[i] / bucketFrequency;
			m_smoother[i].init(sampleRate);
		}
	};
	inline void set(const float attackTimeMS, const float releaseTimeMS) noexcept
	{
		// Set bands smoother
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const float multiplier = 1.0f + (float)(BANDS_COUNT - i - 1) * 0.5f;
			m_smoother[i].set(multiplier * attackTimeMS, multiplier * releaseTimeMS);
		}
	}
	inline void process(const float sample) noexcept
	{
		m_fifo[m_fifoIndex] = sample;
		m_fifoIndex++;

		if (m_fifoIndex == FFT_SIZE)
		{
			juce::zeromem(m_fftData, sizeof(m_fftData));
			memcpy(m_fftData, m_fifo, sizeof(m_fifo));

			_getSpectrum();

			m_fifoIndex = 0;
		}

		// Apply smoother
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			m_spectrumGainsSmooth[i] = m_smoother[i].process(m_spectrumGains[i]);
		}
	}
	inline void release() noexcept
	{
		std::fill(std::begin(m_fifo), std::end(m_fifo), 0.0f);
		std::fill(std::begin(m_fftData), std::end(m_fftData), 0.0f);

		m_fifoIndex = 0;
		m_sampleRate = 48000;
	}
	inline float* getSpectrum()
	{
		return m_spectrumGainsSmooth;
	}

private:
	inline void _getSpectrum() noexcept
	{
		// Apply windowing function
		m_window.multiplyWithWindowingTable(m_fftData, FFT_SIZE);

		// Perform FFT
		m_forwardFFT.performFrequencyOnlyForwardTransform(m_fftData);

		// Get average for each band
		float avg{ 0.0f };
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const auto indexLeft = m_bucketIndex[i];
			const auto indexRight = m_bucketIndex[i + 1];

			float sum{ 0.0f };

			for (size_t j = indexLeft; j < indexRight; j++)
			{
				sum += m_fftData[j];
			}

			sum /= (float)(indexRight - indexLeft);
			//sum *= GAINS_ADJUSTION[i];
			
			m_spectrumGains[i] = sum;
			avg += sum;
		}

		if (avg > 0.001f)
		{
			avg /= (float)BANDS_COUNT;

			// Normalize
			for (size_t i = 0; i < BANDS_COUNT; i++)
			{
				m_spectrumGains[i] /= avg;
			}
		}
		else
		{
			for (size_t i = 0; i < BANDS_COUNT; i++)
			{
				m_spectrumGains[i] = 1.0f;
			}
		}
	}

	juce::dsp::FFT m_forwardFFT;
	juce::dsp::WindowingFunction<float> m_window;

	BranchingEnvelopeFollower<float> m_smoother[BANDS_COUNT];

	float m_fifo[FFT_SIZE]{	0.0f };
	float m_fftData[2 * FFT_SIZE]{ 0.0f };

	float m_spectrumGains[BANDS_COUNT]{ 1.0f };
	float m_spectrumGainsSmooth[BANDS_COUNT]{ 1.0f };
	size_t m_bucketIndex[BANDS_COUNT + 1]{ 0 };

	int m_fifoIndex = 0;
	int m_sampleRate = 48000;
};

class SpectrumDetectionTD
{
public:
	SpectrumDetectionTD() = default;
	~SpectrumDetectionTD() = default;

	static const int BANDS_COUNT = 6;
	inline static float FILTER_FREQUENCY[BANDS_COUNT] = { 50.0f, 120.0f, 380.0f, 1000.0f, 3300.0f, 10000.0f };
	inline static float DETECTION_FILTER_Q[BANDS_COUNT] = { 0.8f, 1.0f, 1.25f, 1.45f, 1.45f, 1.2f };


	inline void init(const int sampleRate) noexcept
	{
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			m_detectionFilter[i].init(sampleRate);
			m_detectionFilter[i].setBandPassPeakGain(FILTER_FREQUENCY[i], DETECTION_FILTER_Q[i]);
			m_smoother[i].init(sampleRate);
		}
	}
	inline void set(const float attackTimeMS, const float releaseTimeMS) noexcept
	{
		// Set bands smoother
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const float multiplier = 1.0f + (float)(BANDS_COUNT - i - 1) * 0.5f;
			m_smoother[i].set(multiplier * attackTimeMS, multiplier * releaseTimeMS);
		}
	}
	inline void process(const float in) noexcept
	{
		// Get bands RMS
		float avg = 0.0f;
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const float filtered = m_detectionFilter[i].processDF1(in);
			const float filteredSmooth = m_smoother[i].process(filtered);

			m_spectrumGainsSmooth[i] = filteredSmooth;
			avg += filteredSmooth;
		}

		if (avg > 0.001f)
		{
			avg /= (float)BANDS_COUNT;

			// Normalize
			for (size_t i = 0; i < BANDS_COUNT; i++)
			{
				m_spectrumGainsSmooth[i] /= avg;
			}
		}
		else
		{
			for (size_t i = 0; i < BANDS_COUNT; i++)
			{
				m_spectrumGainsSmooth[i] = 1.0f;
			}
		}
	}

	float* getSpectrum()
	{
		return m_spectrumGainsSmooth;
	}

private:
	BiquadFilter m_detectionFilter[BANDS_COUNT];
	BranchingEnvelopeFollower<float> m_smoother[BANDS_COUNT];

	float m_spectrumGainsSmooth[BANDS_COUNT]{ 0.0f };
};

class SpectrumMatch
{
public:
	enum DetectioType
	{
		TimeDomain,
		FrequencyDomain,
		COUNT
	};

	struct Params
	{
		static constexpr int NUM_BANDS = 6;

		Params() = default;
		~Params() = default;

		// Constructor with 6 separate float gains
		Params(float attackTimeMS,
			float releaseTimeMS,
			float gain1,
			float gain2,
			float gain3,
			float gain4,
			float gain5,
			float gain6,
			int detectionType,
			bool mute1,
			bool mute2,
			bool mute3,
			bool mute4,
			bool mute5,
			bool mute6)
			: m_attackTimeMS{ attackTimeMS },
			m_releaseTimeMS{ releaseTimeMS },
			m_gains{ gain1, gain2, gain3, gain4, gain5, gain6 },
			m_mute{ mute1, mute2, mute3, mute4, mute5, mute6 },
			m_detectionType{ static_cast<DetectioType>(detectionType) }
		{}

		// Equality helper for floats
		static bool nearlyEqual(float a, float b, float epsilon = 1e-6f)
		{
			return std::fabs(a - b) < epsilon;
		}

		// Equality operator
		bool operator==(const Params& other) const noexcept
		{
			if (!nearlyEqual(m_attackTimeMS, other.m_attackTimeMS) ||
				!nearlyEqual(m_releaseTimeMS, other.m_releaseTimeMS))
				return false;

			for (int i = 0; i < NUM_BANDS; ++i)
			{
				if (!nearlyEqual(m_gains[i], other.m_gains[i]))
					return false;

				if (m_mute[i] != other.m_mute[i])
					return false;
			}

			if (m_detectionType != other.m_detectionType)
			{
				return false;
			}

			return true;
		}

		// Not-equal operator
		bool operator!=(const Params& other) const noexcept
		{
			return !(*this == other);
		}

		float m_attackTimeMS{ 0.0f };
		float m_releaseTimeMS{ 0.0f };
		std::array<float, NUM_BANDS> m_gains{ {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };
		std::array<bool, NUM_BANDS> m_mute{ {false, false, false, false, false, false} };
		DetectioType m_detectionType{ DetectioType::TimeDomain };
	};

	SpectrumMatch() = default;
	~SpectrumMatch() = default;

	static const int BANDS_COUNT = 6;
	inline static float FILTER_FREQUENCY[BANDS_COUNT] = { 50.0f, 120.0f, 380.0f, 1000.0f, 3300.0f, 10000.0f };
	inline static float APPLY_FILTER_Q[BANDS_COUNT] = { 1.0f, 1.0f, 1.0f, 0.7f, 0.3, 0.3f };
	inline static float APPLY_FILTER_SCALE_FACTOR[BANDS_COUNT] = { 1.0f, 1.0f, 0.67f, 0.5f, 0.5f, 0.5f };
	
	inline void init(const int sampleRate) noexcept
	{
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			m_applyfilter[i].init(sampleRate);
		}

		m_spectrumDetectionTD.init(sampleRate);
		m_spectrumDetectionFFT.init(sampleRate);
	}
	inline void set(const Params& params) noexcept
	{
		if (params == m_paramsLast)
		{
			return;
		}

		m_paramsLast = params;

		m_spectrumDetectionTD.set(params.m_attackTimeMS, params.m_releaseTimeMS);
		m_spectrumDetectionFFT.set(params.m_attackTimeMS, params.m_releaseTimeMS);
	}
	inline float process(const float in) noexcept
	{
		float* spectrumGains{ nullptr };
		if (m_paramsLast.m_detectionType == DetectioType::TimeDomain)
		{
			m_spectrumDetectionTD.process(in);
			spectrumGains = m_spectrumDetectionTD.getSpectrum();
		}
		else if (m_paramsLast.m_detectionType == DetectioType::FrequencyDomain)
		{
			m_spectrumDetectionFFT.process(in);
			spectrumGains = m_spectrumDetectionFFT.getSpectrum();
		}
		
		// Apply filters
		float out = in;
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			if (m_paramsLast.m_mute[i] == true)
			{
				m_filterGain[i] = 0.0f;
				continue;
			}
			
			const float gain = spectrumGains[i];
			m_filterGain[i] = m_paramsLast.m_gains[i] - juce::Decibels::gainToDecibels(gain);
			m_applyfilter[i].setPeak(FILTER_FREQUENCY[i], APPLY_FILTER_Q[i], APPLY_FILTER_SCALE_FACTOR[i] * m_filterGain[i]);
			out = m_applyfilter[i].processDF1(out);
		}

		//Out
		return out;
	}
	inline float* getGains() noexcept
	{
		return m_filterGain;
	}

private:
	SpectrumDetectionFFT m_spectrumDetectionFFT{};
	SpectrumDetectionTD m_spectrumDetectionTD{};
	
	Params m_paramsLast{};

	BiquadFilter m_applyfilter[BANDS_COUNT];
	float m_filterGain[BANDS_COUNT]{ 0.0f };

	BranchingEnvelopeFollower<float> m_smoother[BANDS_COUNT];
};
