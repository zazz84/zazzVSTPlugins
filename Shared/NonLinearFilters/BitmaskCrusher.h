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

#include <cstdint>

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class BitmaskCrusher
{
public:
	BitmaskCrusher() = default;
	~BitmaskCrusher() = default;
	
	inline void set(const float threshold, const std::int8_t bitMask)
	{
		m_normalize = 1.0f / threshold;
		const float gainCompensation = 127.0f / bitMask;
		m_denormalize = (1.0f / 128.0f) * threshold * gainCompensation;
		m_bitMask = bitMask;
	}
	inline float process(const float in)
	{		
		// Normalize to range -1.0f - 1.0f
		const float normalized = Math::clamp(m_normalize * in, -1.0f, 1.0f);

		// Convert the sample to a fixed-point integer representation
		std::int8_t fixedPoint = static_cast<std::int8_t>(normalized * 127.0f);

		// Offset
		std::int8_t offset = fixedPoint < 0 ? -m_bitMask : 0;

		// Apply the bitmask
		fixedPoint &= m_bitMask;

		// Add offset
		fixedPoint += offset;

		// Convert back to floating-point representation and denormalize
		return (m_denormalize * static_cast<float>(fixedPoint));
	}
	inline void release()
	{
		m_normalize = 1.0;
		m_denormalize = 1.0f;
		m_bitMask = 0;
	}

private:
	float m_normalize = 1.0;
	float m_denormalize = 1.0f;
	std::int8_t m_bitMask = 0;
};
