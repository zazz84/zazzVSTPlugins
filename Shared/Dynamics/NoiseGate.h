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
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class NoiseGate
{
public:
	NoiseGate() = default;
	~NoiseGate() = default;

	inline void init(const int sampleRate) noexcept
	{
		m_envlopeFollower.init(sampleRate);
	};
	inline void set(const float attack, const float release, const float hold, const float thresholddB, const float hystersisdB = 0.0f) noexcept
	{
		m_envlopeFollower.set(attack, release, hold);
	
		m_thresholdOpen = Math::dBToGain(thresholddB + hystersisdB);
		m_thresholdClose = Math::dBToGain(thresholddB - hystersisdB);
	};
	inline float process(const float in) noexcept
	{
		const float inAbs = Math::fabsf(in);

		float gatedGain = 1.0f;
		if (m_isOpen)
		{
			if (inAbs < m_thresholdClose)
			{
				gatedGain = 0.0f;
			}
			/*else
			{
				gated = 1.0f;
			}*/
		}
		else
		{
			if (inAbs < m_thresholdOpen)
			{
				gatedGain = 0.0f;
			}
			/*else
			{
				gated = 1.0f;
			}*/
		}
		
		const float smoothGain = m_envlopeFollower.process(gatedGain);

		return smoothGain * in;
	};
	inline void release() noexcept
	{
		//m_envlopeFollower.release();
		m_thresholdOpen = 1.0f;
		m_thresholdClose = 1.0f;
		m_isOpen = false;
	}

protected:
	HoldEnvelopeFollower<float> m_envlopeFollower;
	float m_thresholdOpen = 1.0f;
	float m_thresholdClose = 1.0f;
	bool m_isOpen = false;
};
