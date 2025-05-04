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
#include "../../../zazzVSTPlugins/Shared/Reverbs/RoomEarlyReflection.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class SmallRoomReverb
{
public:
	SmallRoomReverb() = default;
	~SmallRoomReverb() = default;

	static const int N_ALLPASSES = 6;
	static const int BUFFER_MINIMUM_SIZE = 2;
	static const int MAX_CHANNELS = 2;

	static constexpr float ALLPASS_DELAY_TIMES_MS[N_ALLPASSES] =	{
																		6.35417f,
																		16.97917f,
																		23.62500f,
																		30.39583f,
																		39.68750f,
																		52.58333f
																	};

	static constexpr float G1 = 0.0f;
	static constexpr float G2 = 0.4f;
	static constexpr float G3 = 0.8f;
	static constexpr float G4 = 0.45f;
	static constexpr float G5 = 0.65f;
	
	static constexpr float  ALLPASS_DELAY_WIDTH[5][N_ALLPASSES] =	{
																		{ G3, G2, G1, G3, G2, G1 },
																		{ G1, G3, G2, G1, G3, G2 },
																		{ G2, G1, G3, G2, G1, G3 },
																		{ G4, G5, G1, G5, G4, G1 },
																		{ G1, G4, G5, G1, G5, G4 }
																	};

	inline void init(const int sampleRate, const int channel) noexcept
	{
		m_sampleRateMS = 0.001f * (float)sampleRate;
		m_channel = channel;

		for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
		{
			const int size = BUFFER_MINIMUM_SIZE + (int)(ALLPASS_DELAY_TIMES_MS[allpass] * m_sampleRateMS);
			m_allpass[allpass].init(size);
		}

		m_ERfilter.init(sampleRate);
		m_LRfilter.init(sampleRate);

		// ER predelay fixed to max 10.0 ms
		// ER lenght fixed to max 60.0 ms
		// LR predelay fixed to max 80.0 ms
		const int size = (int)((float)sampleRate * 0.001f * 80.0f);

		m_earlyReflections.init(size, channel);
	}
	inline void set(const float earlyReflectionsPredelay, const float earlyReflectionsMS, const float earlyReflectionsDamping, const float earlyReflectionsWidth, const float earlyReflectionsGain,
					const float lateReflectionsPredelay, const float lateReflectionsSize, const float lateReflectionsDamping, const float lateReflectionsWidth, const float lateReflectionsGain ) noexcept
	{
		m_ERgain = earlyReflectionsGain;
		m_LRgain = lateReflectionsGain * 7.94f;		// + 18dB gain compensation
		
		// Set early reflections
		m_earlyReflections.set(	(int)(m_sampleRateMS * earlyReflectionsPredelay),
								(int)(m_sampleRateMS * earlyReflectionsMS),
								earlyReflectionsWidth * earlyReflectionsWidth);

		// Set late reflections
		m_LRPredelaySize = 1 + (int)(m_sampleRateMS * lateReflectionsPredelay);
		
		const auto timeFactor = (0.05f + 0.95f * lateReflectionsSize) * m_sampleRateMS;
		const auto width = lateReflectionsWidth * lateReflectionsWidth * lateReflectionsWidth;
		auto& ALLPASS_DELAY_WIDTH_CHANNEL = ALLPASS_DELAY_WIDTH[m_channel];
		
		for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
		{
			const int delay = BUFFER_MINIMUM_SIZE + (int)(timeFactor * ALLPASS_DELAY_TIMES_MS[allpass] * (1.0f - ALLPASS_DELAY_WIDTH_CHANNEL[allpass] * width));
			m_allpass[allpass].set(delay);
		}

		// Set damping filters
		const auto ERdamping = sqrtf(earlyReflectionsDamping);
		const auto ERDampingFrequency = 16000.0f - ERdamping * 14000.0f;
		const auto ERDampingQ = 0.707f - ERdamping * 0.207;
		m_ERfilter.set(ERDampingFrequency, ERDampingQ);

		const auto LRdamping = sqrtf(lateReflectionsDamping);
		const auto LRDampingFrequency = 16000.0f - LRdamping * 14000.0f;
		const auto LRDampingQ = 0.707f - LRdamping * 0.207;
		m_LRfilter.set(LRDampingFrequency, LRDampingQ);
	}
	inline float process(const float in) noexcept
	{
		// Process early reflections
		const float ERout = m_ERfilter.process(m_earlyReflections.process(in));

		// Process serial allpass filters
		float LROut = m_LRfilter.process(m_earlyReflections.readDelay(m_LRPredelaySize));

		m_allpass[0].processReplace(LROut);
		m_allpass[1].processReplace(LROut);
		m_allpass[2].processReplace(LROut);
		m_allpass[3].processReplace(LROut);
		m_allpass[4].processReplace(LROut);
		m_allpass[5].processReplace(LROut);

		//Out
		return m_ERgain * ERout + m_LRgain * LROut;
	}
	inline void release() noexcept
	{
		for (int allpass = 0; allpass < N_ALLPASSES; allpass++)
		{
			m_allpass[allpass].release();
		}

		m_earlyReflections.release();
		m_ERfilter.release();
		m_LRfilter.release();

		m_sampleRateMS = 48.0f;
		m_ERgain = 1.0f;
		m_LRgain = 1.0f;
		m_LRPredelaySize = 0;
		m_channel = 0;
	}

private:
	AllPassFilterSimple m_allpass[N_ALLPASSES];
	RoomEarlyReflectionsSimple m_earlyReflections;
	LowPassBiquadFilter m_ERfilter;
	LowPassBiquadFilter m_LRfilter;
	float m_sampleRateMS = 48.0f;
	float m_ERgain = 1.0f;
	float m_LRgain = 1.0f;
	int m_LRPredelaySize = 0;
	int m_channel = 0;
};