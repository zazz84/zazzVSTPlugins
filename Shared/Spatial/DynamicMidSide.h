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

#include <array>

#include "../../../zazzVSTPlugins/Shared/Dynamics/EnvelopeFollowers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class DynamicMidSide
{
public:
	DynamicMidSide() = default;
	~DynamicMidSide() = default;

	static const int N_CHANNELS = 2;

	inline void init(int sampleRate)
	{
		m_envelope[0].init(sampleRate);
		m_envelope[1].init(sampleRate);
	};
	inline void set(float attack, float release, float mGain, float sGain, float mPan, float sPan, float width = 100.0f)
	{
		m_envelope[0].set(attack, release);
		m_envelope[1].set(attack, release);
		
		m_mGain = mGain;
		m_sGain = sGain;
		m_mPanL = 0.5f - 0.5f * mPan;
		m_mPanR = 0.5f + 0.5f * mPan;
		m_sPanL = 0.5f - 0.5f * sPan;
		m_sPanR = 0.5f + 0.5f * sPan;

		// TODO: Better solution. Width hase to be 50, so left and right are centered the most in some cases
		m_width =  width;
	};
	inline void process(float& left, float& right)
	{
		const float lEnvelope = m_envelope[0].process(left);
		const float rEnvelope = m_envelope[1].process(right);

		float lNormalize = 1.0f;
		float rNormalize = 1.0f;

		if (lEnvelope > 0.0001f && rEnvelope > 0.0001f)
		{
			lNormalize = rEnvelope / lEnvelope;
			rNormalize = lEnvelope / rEnvelope;
		}

		const float lNormalized = left * lNormalize;
		const float rNormalized = right * rNormalize;

		const float mid = m_mGain * (lNormalized + rNormalized);
		const float side = m_sGain * (lNormalized - rNormalized);

		const float norm = Math::remap(m_width, 0.0f, 100.0f, 1.0f, rNormalize);

		left = norm * (m_mPanL * mid + m_sPanL * side);
		right = (m_mPanR * mid - m_sPanR * side) / norm;
	};

private:
	std::array<BranchingEnvelopeFollower<float>, N_CHANNELS> m_envelope;

	float m_mGain = 1.0f;
	float m_sGain = 1.0f;
	float m_mPanL = 1.0f;
	float m_mPanR = 1.0f;
	float m_sPanL = 1.0f;
	float m_sPanR = 1.0f;

	float m_width = 100.0f;
};
