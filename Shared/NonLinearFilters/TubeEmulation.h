/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class TubeEmulation
{
public:
	inline static float process(float in) noexcept
	{
		float out = in;

		// Soft clipper
		constexpr float softClipperRatio = 0.3f;		// The small, the more hard clipping
		constexpr float softClipperThreshold = 0.8913;	// -1dB

		if (const float inAbs = Math::fabsf(in); inAbs > softClipperThreshold)
		{
			const float sign = Math::sign(in);
			out = softClipperThreshold + (inAbs - softClipperThreshold) * softClipperRatio;
			out *= sign;
		}
			
		// ARRY waveshaper
		out = out * (1.0f - (1.0f / 3.0f) * (out * out));

		return out;
	};
};