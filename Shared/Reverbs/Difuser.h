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

#include "../../../zazzVSTPlugins/Shared/Delays/AllPassFilter.h"

class Difuser
{
public:
	Difuser() = default;
	~Difuser() = default;

	static const int MAX_ALLPASS_FILTER_COUNT = 4;

	static constexpr float ALLPASS_FILTER_DELAY_TIME_MS[4][4] = { 
																	{ 3.15f, 5.79f,  0.00f,  0.00f },
																	{ 6.96f, 0.00f,  0.00f,  0.00f },
																	{ 4.77f, 3.59f, 12.73f,  9.30f },
																	{ 3.09f, 3.79f,  8.96f, 11.83f }
																};

	static constexpr float ALLPASS_FILTER_FEEDBACK[4][4] = {
																{ 0.68f, 0.68f, 0.70f, 0.70f },
																{ 0.60f, 0.60f, 0.70f, 0.70f },
																{ 0.75f, 0.75f, 0.63f, 0.63f },
																{ 0.70f, 0.70f, 0.65f, 0.65f }
	};

	static constexpr float ALLPASS_FILTER_DELAY_TIME_MAX_MS[] = {	  6.96f, 5.79f, 12.73f, 11.83f };
	static constexpr int ALLPASS_FILTER_COUNT[] = { 2, 1, 4, 4 };

	enum Type
	{
		Schroeder,
		Moorer,
		Griesinger,
		Zazz
	};

	inline void init(const int sampleRate)
	{
		m_sampleRateMS = 0.001f * (float)sampleRate;

		for (int i = 0; i < MAX_ALLPASS_FILTER_COUNT; i++)
		{
			m_allPassFilter[i].init((int)(ALLPASS_FILTER_DELAY_TIME_MAX_MS[i] * m_sampleRateMS));
		}
	}
	inline void set(const Type type) noexcept
	{
		const int typeIdx = static_cast<int>(type);
		m_allPassFilterCount = ALLPASS_FILTER_COUNT[typeIdx];

		for (int i = 0; i < m_allPassFilterCount; i++)
		{
			m_allPassFilter[i].set((int)(ALLPASS_FILTER_DELAY_TIME_MS[typeIdx][i] * m_sampleRateMS));
			m_allPassFilter[i].setFeedback(ALLPASS_FILTER_FEEDBACK[typeIdx][i]);
		}
	}
	inline float process(const float in) noexcept
	{
		float out = in;
		
		for (int i = 0; i < m_allPassFilterCount; i++)
		{
			out = m_allPassFilter[i].process(out);
		}

		return out;
	}
	inline void release()
	{
		for (int i = 0; i < MAX_ALLPASS_FILTER_COUNT; i++)
		{
			m_allPassFilter[i].release();
		}
	}

private:
	AllPassFilter m_allPassFilter[MAX_ALLPASS_FILTER_COUNT];
	float m_sampleRateMS;
	int m_allPassFilterCount = 0;
};
