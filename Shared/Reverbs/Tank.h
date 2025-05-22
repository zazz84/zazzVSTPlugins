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
#include "../../../zazzVSTPlugins/Shared/Delays/CombFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

//==============================================================================
struct TankParams
{
	enum Type
	{
		Schroeder,
		Moorer,
		Griesinger,
		Zazz
	};

	float predelay = 0.0f;		// ms
	float length = 0.0f;		// ms	
	float size = 0.0f;			
	float damping = 0.0f;
	float width = 0.0f;
	Type type = Type::Schroeder;
	//int type = 0;

	bool operator==(const TankParams& l) const
	{
		return	Math::almostEquals(predelay, l.predelay, 0.01f) &&
				Math::almostEquals(length, l.length, 0.01f) &&
				Math::almostEquals(size, l.size, 0.01f) &&
				Math::almostEquals(damping, l.damping, 0.01f) &&
				type == l.type;
	};
};

//==============================================================================

class Tank
{
public:
	Tank() = default;
	~Tank() = default;

	// Comb filter setup
	static const int COMB_FILTER_COUNT = 6;
	static constexpr float SCHROEDER_COMP_FILTER_DELAY_TIME_MS[] =	{ 30.14f, 34.64f, 39.72f, 45.89f };
	static constexpr float MOORER_COMB_FILTER_DELAY_TIME_MS[] =		{ 39.86f, 44.19f, 47.91f, 51.99f, 55.94f, 60.02f };
	static constexpr float ZAZZ_COMB_FILTER_DELA_TIMEY_MS[] =		{ 30.07f, 48.02f, 60.10f, 72.96f };
	static constexpr float COMB_FILTER_DELAY_TIME_MAX_MS[] =		{ 39.86f, 48.02f, 60.10f, 72.96f, 55.94f, 60.02f };

	// Other
	static constexpr float GREISINGER_ALLPASS_DELAY_TIME_MS[] = {  22.5799f,  60.4818f,  30.5097f,  89.2443f };
	static constexpr float GREISINGER_DELAY_TIME_MS[] =			{ 149.6253f, 124.9958f, 141.6955f, 105.3728f };
	static constexpr float GREISINGER_TAP_LEFT_TIME_MS[] =		{   7.5938f,  99.9294f,  64.2787f,  67.0676f, 66.8660f,  6.2834f, 35.8187f };
	static constexpr float GREISINGER_TAP_RIGHT_TIME_MS[] =		{  11.8612f, 121.8709f,  41.2621f,  89.8155f, 70.9318f, 11.9284f,  4.0657f };

	// Gain compensation
	static constexpr float GAIN_COMPENSATION_MIN[] = {  -6.1f,  -7.9f,  3.1f, -5.3f };
	static constexpr float GAIN_COMPENSATION_MAX[] = { -10.2f, -10.8f, -6.1f, -11.7f };

	inline void init(const int sampleRate)
	{
		m_sampleRateMS = 0.001f * (float)sampleRate;

		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			const int size = (int)(COMB_FILTER_DELAY_TIME_MAX_MS[i] * m_sampleRateMS);
			m_combFilter[i].init(size, sampleRate);
		}

		// Greisinger
		m_appPassFilter[0].init((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[0] * m_sampleRateMS));
		m_appPassFilter[1].init((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[1] * m_sampleRateMS));
		m_appPassFilter[2].init((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[2] * m_sampleRateMS));
		m_appPassFilter[3].init((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[3] * m_sampleRateMS));

		m_delayLine[0].init((int)(GREISINGER_DELAY_TIME_MS[0] * m_sampleRateMS));
		m_delayLine[1].init((int)(GREISINGER_DELAY_TIME_MS[1] * m_sampleRateMS));
		m_delayLine[2].init((int)(GREISINGER_DELAY_TIME_MS[2] * m_sampleRateMS));
		m_delayLine[3].init((int)(GREISINGER_DELAY_TIME_MS[3] * m_sampleRateMS));

		m_dampingFilter[0].init(sampleRate);
		m_dampingFilter[1].init(sampleRate);

		m_appPassFilter[0].setFeedback(0.7f);
		m_appPassFilter[1].setFeedback(0.7f);
		m_appPassFilter[2].setFeedback(0.65f);
		m_appPassFilter[3].setFeedback(0.65f);
	};
	inline void set(TankParams& params) noexcept
	{
		if (m_params == params)
		{
			return;
		}

		m_params = params;

		const int typeIdx = static_cast<int>(params.type);
		const float rt60 = Math::remap(params.length, 0.0f, 1.0f, 0.1f, 3.0f);
		const float sampleRate = 1000.0f * m_sampleRateMS;
		

		m_gainCompensation = juce::Decibels::decibelsToGain(Math::remap(params.length, 0.0f, 1.0f, GAIN_COMPENSATION_MIN[typeIdx], GAIN_COMPENSATION_MAX[typeIdx]));

		if (params.type == TankParams::Type::Schroeder)
		{	
			const float sizeFactor = 0.25f + params.size * 0.75f;

			for (int i = 0; i < 4; i++)
			{
				const int size = (int)(SCHROEDER_COMP_FILTER_DELAY_TIME_MS[i] * m_sampleRateMS * sizeFactor);
				m_combFilter[i].setSize(size);
				m_combFilter[i].setDamping(0.0f);
				m_combFilter[i].setTime(rt60, sampleRate);
			}
		}
		else if (params.type == TankParams::Type::Moorer)
		{
			const float sizeFactor = 0.25f + params.size * 0.75f;

			for (int i = 0; i < 6; i++)
			{
				const int size = (int)(MOORER_COMB_FILTER_DELAY_TIME_MS[i] * m_sampleRateMS * sizeFactor);
				m_combFilter[i].setSize(size);
				m_combFilter[i].setDamping(0.9f * params.damping);
				m_combFilter[i].setTime(rt60, sampleRate);
			}
		}
		else if (params.type == TankParams::Type::Griesinger)
		{
			m_decay = Math::remap(params.length, 0.0f, 1.0f, juce::Decibels::decibelsToGain(-60.0f), juce::Decibels::decibelsToGain(-3.0f));

			const float sizeFactor = 0.1f + params.size * 0.9f;

			m_appPassFilter[0].set((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[0] * m_sampleRateMS * sizeFactor));
			m_appPassFilter[1].set((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[1] * m_sampleRateMS * sizeFactor));
			m_appPassFilter[2].set((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[2] * m_sampleRateMS * sizeFactor));
			m_appPassFilter[3].set((int)(GREISINGER_ALLPASS_DELAY_TIME_MS[3] * m_sampleRateMS * sizeFactor));

			m_delayLine[0].set((int)(GREISINGER_DELAY_TIME_MS[0] * m_sampleRateMS * sizeFactor));
			m_delayLine[1].set((int)(GREISINGER_DELAY_TIME_MS[1] * m_sampleRateMS * sizeFactor));
			m_delayLine[2].set((int)(GREISINGER_DELAY_TIME_MS[2] * m_sampleRateMS * sizeFactor));
			m_delayLine[3].set((int)(GREISINGER_DELAY_TIME_MS[3] * m_sampleRateMS * sizeFactor));

			m_dampingFilter[0].setCoef(0.9f * params.damping);
			m_dampingFilter[1].setCoef(0.9f * params.damping);
		}
		else if (params.type == TankParams::Type::Zazz)
		{
			const float sizeFactor = 0.2f + params.size * 0.8f;

			for (int i = 0; i < 4; i++)
			{
				const int size = (int)(ZAZZ_COMB_FILTER_DELA_TIMEY_MS[i] * m_sampleRateMS * sizeFactor);
				m_combFilter[i].setSize(size);
				m_combFilter[i].setDamping(0.9f * params.damping);
				m_combFilter[i].setTime(rt60, sampleRate);
			}
		}
	};
	inline float process(const float in) noexcept
	{
		float out = 0.0f;
		
		if (m_params.type == TankParams::Type::Schroeder)
		{
			for (int i = 0; i < 4; i++)
			{
				out += m_combFilter[i].process(in);
			}
		}
		else if (m_params.type == TankParams::Type::Moorer)
		{
			for (int i = 0; i < 6; i++)
			{
				out += m_combFilter[i].process(in);
			}
		}
		else if (m_params.type == TankParams::Type::Griesinger)
		{
			// Left tank
			float leftTankOut = in + m_decay * m_delayLine[3].read();

			leftTankOut = m_appPassFilter[0].process(leftTankOut);

			m_delayLine[0].write(leftTankOut);
			leftTankOut = m_delayLine[0].read();

			leftTankOut = m_decay * m_dampingFilter[0].process(leftTankOut);
			leftTankOut = m_appPassFilter[1].process(leftTankOut);

			m_delayLine[1].write(leftTankOut);

			// Right tank
			float rightTankOut = in + m_decay * m_delayLine[1].read();

			rightTankOut = m_appPassFilter[2].process(m_decay * rightTankOut);

			m_delayLine[2].write(rightTankOut);
			rightTankOut = m_delayLine[2].read();

			rightTankOut = m_decay * m_dampingFilter[1].process(rightTankOut);
			rightTankOut = m_appPassFilter[3].process(rightTankOut);

			m_delayLine[3].write(rightTankOut);

			const float sizeFactor = 0.3f + m_params.size * 0.7f;

			//Taps
			// Note: Reading from buffers for left channel, but using delay times for right channel. Seem it produces more smooth reverberation with less noticable echoes.
			out  = m_delayLine[2].readDelay(		(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[0] * m_sampleRateMS));
			out += m_delayLine[2].readDelay(		(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[1] * m_sampleRateMS));
			out -= m_appPassFilter[3].readDelay(	(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[2] * m_sampleRateMS));
			out += m_delayLine[3].readDelay(		(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[3] * m_sampleRateMS));
			out -= m_delayLine[0].readDelay(		(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[4] * m_sampleRateMS));
			out -= m_appPassFilter[1].readDelay(	(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[5] * m_sampleRateMS));
			out -= m_delayLine[1].readDelay(		(int)(sizeFactor * GREISINGER_TAP_RIGHT_TIME_MS[6] * m_sampleRateMS));
		}
		else if (m_params.type == TankParams::Type::Zazz)
		{
			for (int i = 0; i < 4; i++)
			{
				out += m_combFilter[i].process(in);
			}
		}

		return m_gainCompensation * out;
	};
	inline void release()
	{
		for (int i = 0; i < COMB_FILTER_COUNT; i++)
		{
			m_combFilter[i].release();
		}

		for (int i = 0; i < 4; i++)
		{
			m_appPassFilter[i].release();
			m_delayLine[i].release();
		}

		m_dampingFilter[0].release();
		m_dampingFilter[1].release();

		m_sampleRateMS = 48.0f;
		m_decay = 0.5f;
	};

private:
	LowPassCombFilter m_combFilter[COMB_FILTER_COUNT];
	AllPassFilter m_appPassFilter[4];
	CircularBuffer m_delayLine[4];
	OnePoleLowPassFilter m_dampingFilter[2];

	TankParams m_params = {};
	
	float m_sampleRateMS = 48.0f;
	float m_decay = 0.5f;
	float m_gainCompensation;
};