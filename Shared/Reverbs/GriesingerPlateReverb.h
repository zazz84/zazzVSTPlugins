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

// https://freeverb3-vst.sourceforge.io/doc/Plate_Reverb.PDF
// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
// https://github.com/antonhornquist/jon-pd

class GriesingerPlateReverb
{
public:
	GriesingerPlateReverb() = default;
	~GriesingerPlateReverb() = default;

	static constexpr float DIFUSER_DELAY_TIME_MS[] = { 4.77134f, 3.59530f, 12.7348f, 9.30748f };
	static constexpr float TANK_DIFUSER_TIME_MS[] = { 22.57988f, 60.48183f, 30.50972f, 89.24431f };
	static constexpr float TANK_DELAY_TIME_MS[] = { 149.62534f, 124.99579f, 141.69550f, 106.28003f };
	static constexpr float TAP_LEFT_TIME_MS[] = { 8.93787f, 99.92943f, 64.27875f, 67.06763f, 66.86603f, 6.28339f, 35.81868f };

	inline void init(const int sampleRate)
	{
		m_sampleRateMS = (float)sampleRate * 0.001f;

		// Init
		// ER predelay fixed to max 10.0 ms
		// ER lenght fixed to max 60.0 ms
		// LR predelay fixed to max 80.0 ms
		m_earlyReflections.init((int)((float)m_sampleRateMS * 80.0f));

		m_difuser[0].init((int)(DIFUSER_DELAY_TIME_MS[0] * m_sampleRateMS));
		m_difuser[1].init((int)(DIFUSER_DELAY_TIME_MS[1] * m_sampleRateMS));
		m_difuser[2].init((int)(DIFUSER_DELAY_TIME_MS[2] * m_sampleRateMS));
		m_difuser[3].init((int)(DIFUSER_DELAY_TIME_MS[3] * m_sampleRateMS));

		m_tankDifuser[0].init((int)(TANK_DIFUSER_TIME_MS[0] * m_sampleRateMS));
		m_tankDifuser[1].init((int)(TANK_DIFUSER_TIME_MS[1] * m_sampleRateMS));
		m_tankDifuser[2].init((int)(TANK_DIFUSER_TIME_MS[2] * m_sampleRateMS));
		m_tankDifuser[3].init((int)(TANK_DIFUSER_TIME_MS[3] * m_sampleRateMS));

		m_tankDelay[0].init((int)(TANK_DELAY_TIME_MS[0] * m_sampleRateMS));
		m_tankDelay[1].init((int)(TANK_DELAY_TIME_MS[1] * m_sampleRateMS));
		m_tankDelay[2].init((int)(TANK_DELAY_TIME_MS[2] * m_sampleRateMS));
		m_tankDelay[3].init((int)(TANK_DELAY_TIME_MS[3] * m_sampleRateMS));

		m_inputFilter.init(sampleRate);

		m_dampingFilter[0].init(sampleRate);
		m_dampingFilter[1].init(sampleRate);

		// Set
		m_difuser[0].setFeedback(0.75f);
		m_difuser[1].setFeedback(0.75f);
		m_difuser[2].setFeedback(0.625f);
		m_difuser[3].setFeedback(0.625f);

		m_tankDifuser[0].setFeedback(0.7f);
		m_tankDifuser[1].setFeedback(0.7f);
		m_tankDifuser[2].setFeedback(0.5f);
		m_tankDifuser[3].setFeedback(0.5f);
	}
	inline void set(const float earlyReflectionsPredelayMS, const float earlyReflectionsSize, const float earlyReflectionsDamping, const float earlyReflectionsWidth, const float earlyReflectionsGain,
		            const float lateReflectionsPredelayMS, const float lateReflectionsSize, const float lateReflectionsDamping, const float lateReflectionsWidth, const float lateReflectionsGain) noexcept
	{
		// Late reflections
		const float LRgainCompensation = juce::Decibels::decibelsToGain(Math::remap(lateReflectionsSize, 0.0f, 1.0f, -5.5f, -10.5f));
		m_LRgain = lateReflectionsGain * LRgainCompensation;

		m_LRPredelaySamples = 4 + (int)(lateReflectionsPredelayMS * m_sampleRateMS);

		m_inputFilter.setCoef(0.65f * lateReflectionsDamping);
		m_dampingFilter[0].setCoef(0.9f * lateReflectionsDamping);
		m_dampingFilter[1].setCoef(0.9f * lateReflectionsDamping);

		m_decay = 0.05f + 0.7f * lateReflectionsSize;
	}
	inline float process(const float in)
	{
		// Early reflections
		m_earlyReflections.write(in);

		// Difuser
		float difuserOut = m_inputFilter.process(m_earlyReflections.readDelay(m_LRPredelaySamples));

		difuserOut = m_difuser[0].process(difuserOut);
		difuserOut = m_difuser[1].process(difuserOut);
		difuserOut = m_difuser[2].process(difuserOut);
		difuserOut = m_difuser[3].process(difuserOut);

		// Left tank
		float leftTankOut = difuserOut + m_decay * m_tankDelay[3].read();

		leftTankOut = m_tankDifuser[0].process(leftTankOut);
		
		m_tankDelay[0].write(leftTankOut);
		leftTankOut = m_tankDelay[0].read();

		leftTankOut = m_decay * m_dampingFilter[0].process(leftTankOut);
		leftTankOut = m_tankDifuser[1].process(leftTankOut);

		m_tankDelay[1].write(leftTankOut);

		// Right tank
		float rightTankOut = difuserOut + m_decay * m_tankDelay[1].read();

		rightTankOut = m_tankDifuser[2].process(rightTankOut);

		m_tankDelay[2].write(rightTankOut);
		rightTankOut = m_tankDelay[2].read();

		rightTankOut = m_decay * m_dampingFilter[1].process(rightTankOut);
		rightTankOut = m_tankDifuser[3].process(rightTankOut);

		m_tankDelay[3].write(rightTankOut);

		//Taps
		float out = 0.0f;
		out  = 0.6f * m_tankDelay[2].readDelay((int)(TANK_DELAY_TIME_MS[0] * m_sampleRateMS));
		out += 0.6f * m_tankDelay[2].readDelay((int)(TANK_DELAY_TIME_MS[1] * m_sampleRateMS));
		out -= 0.6f * m_tankDifuser[3].readDelay((int)(TANK_DELAY_TIME_MS[2] * m_sampleRateMS));
		out += 0.6f * m_tankDelay[3].readDelay((int)(TANK_DELAY_TIME_MS[3] * m_sampleRateMS));
		out -= 0.6f * m_tankDelay[0].readDelay((int)(TANK_DELAY_TIME_MS[4] * m_sampleRateMS));
		out -= 0.6f * m_tankDifuser[1].readDelay((int)(TANK_DELAY_TIME_MS[5] * m_sampleRateMS));

		return m_LRgain * (out - 0.6f * m_tankDelay[1].readDelay((int)(TANK_DELAY_TIME_MS[6] * m_sampleRateMS)));
	}
	inline void release()
	{
		m_earlyReflections.release();

		m_difuser[0].release();
		m_difuser[1].release();
		m_difuser[2].release();
		m_difuser[3].release();

		m_tankDifuser[0].release();
		m_tankDifuser[1].release();
		m_tankDifuser[2].release();
		m_tankDifuser[3].release();

		m_tankDelay[0].release();
		m_tankDelay[1].release();
		m_tankDelay[2].release();
		m_tankDelay[3].release();

		m_inputFilter.release();

		m_dampingFilter[0].release();
		m_dampingFilter[1].release();
	}

private:
	CircularBuffer m_earlyReflections;
	AllPassFilter m_difuser[4];
	AllPassFilter m_tankDifuser[4];
	CircularBuffer m_tankDelay[4];
	OnePoleLowPassFilter m_inputFilter;
	OnePoleLowPassFilter m_dampingFilter[2];
	
	float m_sampleRateMS = 48.0f;
	float m_decay = 0.5f;
	float m_LRgain = 1.0f;
	int m_LRPredelaySamples = 4;
};