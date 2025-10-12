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

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class ZeroCrossingRateOffline
{
public:
	ZeroCrossingRateOffline() = default;
	~ZeroCrossingRateOffline() = default;

	void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		m_filter.init(sampleRate);
	};
	void set(const int estimatedFrequencyMin, const int estimatedFrequencyMax)
	{
		m_estimatedFrequencyMin = estimatedFrequencyMin;
		m_estimatedFrequencyMax = estimatedFrequencyMax;
	};
	float process(const juce::AudioBuffer<float>& audioBuffer)
	{
		m_maxRMS = 0.0f;
		m_detectedFrequency = 0.0f;
		
		const int channels = audioBuffer.getNumChannels();
		const int samples = audioBuffer.getNumSamples();

		const int steps = m_estimatedFrequencyMax - m_estimatedFrequencyMin;
		
		for (int step = 0; step < steps; step++)
		{
			const float frequency = (float)(m_estimatedFrequencyMin + step);

			m_filter.setBandPassPeakGain(frequency, 0.707f);

			for (int channel = 0; channel < channels; channel++)
			{
				auto* pAudioBuffer = audioBuffer.getReadPointer(channel);

				m_filter.reset();

				float sumSquares = 0.0f;

				for (int sample = 0; sample < samples; sample++)
				{
					const float in = pAudioBuffer[sample];
					const float out = m_filter.processDF1(in);

					sumSquares += out * out;
				}

				const float RMS = std::sqrt(sumSquares / samples);

				if (RMS > m_maxRMS)
				{
					m_maxRMS = RMS;
					m_detectedFrequency = frequency;
				}
			}
		}

		return 1.0f / m_detectedFrequency;
	};

private:
	BiquadFilter m_filter;
	int m_estimatedFrequencyMin = 0;
	int m_estimatedFrequencyMax = 0;
	float m_detectedFrequency = 0.0f;
	float m_maxRMS = 0.0f;
	int m_sampleRate = 48000;
};