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

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class ClassBAmplifier
{
public:
	ClassBAmplifier() = default;
	~ClassBAmplifier() = default;

	inline void init(int sampleRate)
	{
		m_dcBlockerPos.init(sampleRate);
		m_dcBlockerNeg.init(sampleRate);
		m_dcBlockerPos.setHighPass(20.0f, 0.707f);
		m_dcBlockerNeg.setHighPass(20.0f, 0.707f);
	};
	inline void set(float drive)
	{
		m_drive = drive;
	}
	inline float process(float in)
	{
		constexpr float positiveLimit = 23.6f;
		constexpr float negativeLimit = 0.5f;

		float posAsym = Waveshapers::Poletti(in, m_drive, positiveLimit, negativeLimit);
		float negAsym = Waveshapers::Poletti(in, m_drive, negativeLimit, positiveLimit);

		posAsym = m_dcBlockerPos.processDF1(posAsym);
		negAsym = m_dcBlockerNeg.processDF1(negAsym);

		constexpr float symetricLimit = 0.5f;
		constexpr float symetricGain = 4.0f;

		posAsym =  Waveshapers::Poletti(posAsym, symetricGain, symetricLimit, symetricLimit);
		negAsym =  Waveshapers::Poletti(negAsym, symetricGain, symetricLimit, symetricLimit);

		return 0.5f * (posAsym + negAsym);
	}

private:
	BiquadFilter m_dcBlockerPos, m_dcBlockerNeg;
	float m_drive;
};
