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

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"

class AdaptiveNoiseGate
{
public:
	AdaptiveNoiseGate() {};
	inline void init(const int sampleRate)
	{
		m_envelopeFollowerSlow.init(sampleRate);
		m_envelopeFollowerFast.init(sampleRate);
		
		m_gateSmoothing.init(sampleRate);

		m_envelopeFollowerSlow.set(25.0f, 75.0f);
		m_envelopeFollowerFast.set(3.0f, 75.0f);
	}
	inline void set(float attackTimeMS, float releaseTimeMS, float sensitivity)
	{
		m_gateSmoothing.set(1.0f, releaseTimeMS, attackTimeMS);

		m_sensitivity = sensitivity;
	}
	inline float process(float in)
	{
		const float envelopeIn = std::fabsf(in);
		// Add 1e-6 to avoid division by 0. Envelope output is alway > 0
		const float envelopeSlow = m_envelopeFollowerSlow.process(envelopeIn);
		const float envelopeFast = m_envelopeFollowerFast.process(envelopeIn);

		// Clamp
		float difference = 1.0f;
			
		if (envelopeSlow > 0.0f)
		{
			difference = std::fminf(8.0f, std::fmaxf(0.1f, envelopeFast / envelopeSlow));
		}		
		
		// Store max for UI
		if (difference > m_difference)
		{
			m_difference = difference;
		}

		//float gain = envelopeIn > m_threshold ? 1.0f : 0.0f;
		float gain = difference > m_sensitivity ? 1.0f : 0.0f;

		// Smooth gate gain
		gain = m_gateSmoothing.process(gain);

		return gain * in;
	}
	inline float getDifference()
	{
		const float difference = m_difference;
		m_difference = 0.0f;
		return difference;
	}

private:
	BranchingEnvelopeFollower<float> m_envelopeFollowerSlow;
	BranchingEnvelopeFollower<float> m_envelopeFollowerFast;
	HoldEnvelopeFollower<float> m_gateSmoothing;

	float m_difference = 0.0f;
	float m_sensitivity = 4.0f;
};