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

class SpectrumMatch
{
public:
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
			float gain6)
			: m_attackTimeMS{ attackTimeMS },
			m_releaseTimeMS{ releaseTimeMS },
			m_gains{ gain1, gain2, gain3, gain4, gain5, gain6 }
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
	};

	SpectrumMatch() = default;
	~SpectrumMatch() = default;

	static const int BANDS_COUNT = 6;
	inline static float FILTER_FREQUENCY[BANDS_COUNT] = { 50.0f, 120.0f, 380.0f, 1000.0f, 3300.0f, 10000.0f };
	inline static float DETECTION_FILTER_Q[BANDS_COUNT] = { 0.8f, 1.0f, 1.25f, 1.45f, 1.45f, 1.2f };
	inline static float APPLY_FILTER_Q[BANDS_COUNT] = { 1.0f, 1.0f, 1.0f, 0.7f, 0.3, 0.3f };
	inline static float APPLY_FILTER_SCALE_FACTOR[BANDS_COUNT] = { 1.0f, 1.0f, 0.67f, 0.5f, 0.5f, 0.5f };
	
	inline void init(const int sampleRate) noexcept
	{
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			m_detectionFilter[i].init(sampleRate);
			m_detectionFilter[i].setBandPassPeakGain(FILTER_FREQUENCY[i], DETECTION_FILTER_Q[i]);

			m_applyfilter[i].init(sampleRate);

			m_smoother[i].init(sampleRate);
		}
	}
	inline void set(const Params& params) noexcept
	{
		if (params == m_paramsLast)
		{
			return;
		}

		m_paramsLast = params;

		// Set bands smoother
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const float multiplier = 1.0f + (float)(BANDS_COUNT - i - 1) * 0.5f;
			m_smoother[i].set(multiplier * params.m_attackTimeMS, multiplier * params.m_releaseTimeMS);
		}
	}
	inline float process(const float in) noexcept
	{
		// Get bands RMS
		float rmsSmoothdB[BANDS_COUNT]{ 0.0f };
		float avgdB = 0.0f;

		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			const float filtered = m_detectionFilter[i].processDF1(in);
			const float rmsSmooth = m_smoother[i].process(filtered);
			const float dB = juce::Decibels::gainToDecibels(rmsSmooth);
			rmsSmoothdB[i] = dB;
			avgdB += dB;
		}

		// Get average RMS
		avgdB /= (float)BANDS_COUNT;

		// Set and apply filter
		float out = in;
		for (size_t i = 0; i < BANDS_COUNT; i++)
		{
			m_filterGain[i] = m_paramsLast.m_gains[i] - rmsSmoothdB[i] + avgdB;
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
	Params m_paramsLast{};

	BiquadFilter m_detectionFilter[BANDS_COUNT];
	BiquadFilter m_applyfilter[BANDS_COUNT];
	float m_filterGain[BANDS_COUNT]{ 0.0f };

	BranchingEnvelopeFollower<float> m_smoother[BANDS_COUNT];
};
