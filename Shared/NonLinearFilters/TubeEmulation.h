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

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

class TubeEmulation
{
public:
	TubeEmulation() = default;
	~TubeEmulation() = default;

	inline void init(int sampleRate)
	{
		m_clipper.init(sampleRate);
	};
	inline float process(float in)
	{
		float out = m_clipper.process(in);
		out = 0.675f * Waveshapers::ARRY(out);
		return out;
	};

private:
	SoftClipper m_clipper;
};