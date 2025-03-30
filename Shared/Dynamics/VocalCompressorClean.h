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

#include "../../../zazzVSTPlugins/Shared/Dynamics/Compressors.h"

class VocalCompressorClean
{
public:
	VocalCompressorClean() = default;
	~VocalCompressorClean() = default;

	inline void init(int sampleRate)
	{
		m_FETCompressor.init(sampleRate);
		m_optoCompressor.init(sampleRate);

		m_FETCompressor.set(-6.0f, 12.0f, 0.0f, 0.1f, 30.0f);
		m_optoCompressor.set(-3.0f, 50.0f, 40.0f, 6.0f, 200.0f);
	};
	inline float process(float in)
	{
		return m_optoCompressor.processSoftKnee(m_FETCompressor.processHardKneeLogPeak(in));
	}

protected:
	Compressor m_FETCompressor;
	OptoCompressor m_optoCompressor;
};
