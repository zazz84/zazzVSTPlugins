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

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/Compressors.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/SideChainCompressor.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/TubeEmulation.h"

class VocalCompressor
{
public:
	VocalCompressor() = default;
	~VocalCompressor() = default;

	inline void init(int sampleRate)
	{
		m_leveler.init(sampleRate);
		m_compressor.init(sampleRate);
		m_tubeEmulation.init(sampleRate);
		
		// Leveler compresor setup
		m_leveler.set(-18.0f, 1.5f, 0.0f, 2000.0f, 3000.0f, 660.0f);

		// Set tube emulation
		m_tubeEmulation.set(-8.0f);

		// Compressor setup
		m_compressor.set(-23.0f, 50.0f, 60.0f, 3.0f, 150.0f);

		// Output gain compensation
		m_gainCompensation = Math::dBToGain(16.0f);
	};
	inline float process(float in)
	{
		float out = m_leveler.processHardKnee(in);
		
		out = m_tubeEmulation.process(out);
		
		out = m_compressor.processSoftKneeLinPeak(out);
		
		return m_gainCompensation * out;
	}

protected:
	SideChainCompressor m_leveler;
	Compressor m_compressor;
	TubeEmulation m_tubeEmulation;
	float m_gainCompensation = 1.0f;
};