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

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class SmallSpeakerSimulation
{
public:
	SmallSpeakerSimulation() = default;
	~SmallSpeakerSimulation() = default;
	
	inline void init(const int sampleRate) noexcept
	{
		m_filter1.init(sampleRate);
		m_filter2.init(sampleRate);
		m_filter3.init(sampleRate);
		m_filter4.init(sampleRate);
		m_filter5.init(sampleRate);
		m_filter6.init(sampleRate);

		m_frequencyMax = (0.9f / 2.0f) * (float)sampleRate;
	};
	inline void set(const int type, const float tune) noexcept
	{
		const float tuneFactor = 0.01f * tune;

		// https://fokkie.home.xs4all.nl/IR.htm
		// Telephone 90 model
		if (type == 1)
		{
			m_gain = juce::Decibels::decibelsToGain(6.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 0.707f);
			m_filter2.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 1300.0f), 1.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 0.7f, -4.5f);
			m_filter4.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3900.0f), 0.52f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3900.0f), 0.71f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3900.0f), 1.93f);
		}
		else if (type == 2)
		{
			m_gain = juce::Decibels::decibelsToGain(3.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 180.0f), 0.5f);
			m_filter2.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 660.0f), 3.5f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 1800.0f), 0.5f, -12.0f);
			m_filter4.setPeak(std::fminf(m_frequencyMax, tuneFactor * 11000.0f), 8.0f, -9.0f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 8000.0f), 1.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 8000.0f), 2.0f);
		}
		else if (type == 3)
		{
			m_gain = juce::Decibels::decibelsToGain(3.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 1100.0f), 2.1f);
			m_filter2.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 1200.0f), 1.1f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 3600.0f), 8.0f, 12.0f);
			m_filter4.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2200.0f), 0.52f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2200.0f), 0.71f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2200.0f), 1.93f);
		}
		else if (type == 4)
		{
			m_gain = 1.0f;

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 500.0f), 2.0f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 1000.0f), 2.0f, -9.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 3800.0f), 6.0f, -9.0f);
			m_filter4.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 6000.0f), 0.7f, -12.0f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 7000.0f), 1.5f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 7000.0f), 4.3f);
		}
		else if (type == 5)
		{
			m_gain = juce::Decibels::decibelsToGain(-1.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 680.0f), 0.65f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 360.0f), 2.8f, 18.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 680.0f), 1.0f, -6.0f);
			m_filter4.setPeak(std::fminf(m_frequencyMax, tuneFactor * 4600.0f), 6.0f, -9.0f);
			m_filter5.setPeak(std::fminf(m_frequencyMax, tuneFactor * 6500.0f), 6.0f, -12.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3000.0f), 4.3f);
		}
		else if (type == 6)
		{
			m_gain = juce::Decibels::decibelsToGain(-4.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 1.0f);
			m_filter2.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 0.7f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 380.0f), 1.0f, 18.0f);
			m_filter4.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 2400.0f), 1.0f, -12.0f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2400.0f), 3.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2500.0f), 1.0f);
		}
		else if (type == 7)
		{
			m_gain = juce::Decibels::decibelsToGain(11.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 600.0f), 3.0f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 0.5f, -18.0f);
			m_filter3.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 2100.0f), 1.0f, -12.0f);
			m_filter4.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3000.0f), 0.52f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3000.0f), 0.71f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 3000.0f), 1.93f);
		}
		else if (type == 8)
		{
			m_gain = juce::Decibels::decibelsToGain(-6.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 500.0f), 2.0f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 560.0f), 1.0f, 9.0f);
			m_filter3.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 710.0f), 10.0f, -9.0f);
			m_filter4.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 1800.0f), 2.0f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 1800.0f), 2.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 1800.0f), 4.0f);
		}
		else if (type == 9)
		{
			m_gain = juce::Decibels::decibelsToGain(-5.0f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 540.0f), 2.5f);
			m_filter2.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 800.0f), 3.0f);
			m_filter3.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 2000.0f), 10.0f, -18.0f);
			m_filter4.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2600.0f), 2.0f);
			m_filter5.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2600.0f), 2.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2600.0f), 4.0f);
		}
		// Telephone 60s
		else if (type == 10)
		{
			m_gain = juce::Decibels::decibelsToGain(-8.8f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 430.0f), 7.0f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 580.0f), 3.0f, -12.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 2250.0f), 3.5f, 18.0f);
			m_filter4.setLowShelf(std::fminf(m_frequencyMax, tuneFactor * 1400.0f), 2.0f, -3.0f);
			m_filter5.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 3000.0f), 4.0f, -18.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 10000.0f), 2.0f);
		}
		// https://fokkie.home.xs4all.nl/IR.htm
		// 70s Philips Box
		else if (type == 11)
		{
			m_gain = juce::Decibels::decibelsToGain(5.5f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 220.0f), 1.0f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 800.0f), 2.0f, -12.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 3700.0f), 9.0f, -9.0f);
			m_filter4.setPeak(std::fminf(m_frequencyMax, tuneFactor * 8000.0f), 5.0f, -9.0f);
			m_filter5.setHighShelf(std::fminf(m_frequencyMax, tuneFactor * 2500.0f), 0.5f, -6.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 12000.0f), 0.8f);
		}
		// Computer speaker
		else if (type == 12)
		{
			m_gain = juce::Decibels::decibelsToGain(7.3f);

			m_filter1.setHighPass(std::fminf(m_frequencyMax, tuneFactor * 330.0f), 0.3f);
			m_filter2.setPeak(std::fminf(m_frequencyMax, tuneFactor * 800.0f), 3.0f, -3.0f);
			m_filter3.setPeak(std::fminf(m_frequencyMax, tuneFactor * 1800.0f), 3.0f, -3.0f);
			m_filter4.setPeak(std::fminf(m_frequencyMax, tuneFactor * 3300.0f), 3.0f, -3.0f);
			m_filter5.setPeak(std::fminf(m_frequencyMax, tuneFactor * 7000.0f), 3.5f, -3.0f);
			m_filter6.setLowPass(std::fminf(m_frequencyMax, tuneFactor * 2800.0f), 1.0f);
		}
	}
	inline float process(const float in) noexcept
	{
		float out = m_filter1.processDF2T(in);
		out = m_filter2.processDF2T(out);
		out = m_filter3.processDF2T(out);
		out = m_filter4.processDF2T(out);
		out = m_filter5.processDF2T(out);
		return m_gain * m_filter6.processDF2T(out);
	};
	inline void release() noexcept
	{
		m_filter1.release();
		m_filter2.release();
		m_filter3.release();
		m_filter4.release();
		m_filter5.release();
		m_filter6.release();

		m_gain = 1.0f;
	};

private:
	BiquadFilter m_filter1, m_filter2, m_filter3, m_filter4, m_filter5, m_filter6;
	float m_gain = 1.0f;
	float m_frequencyMax = 21600.0f;	// 0.9f * sampleRate / 2.0f
};