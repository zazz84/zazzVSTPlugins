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

#include <vector>
#include <cmath>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"

class BitCrusher
{
public:
	BitCrusher() = default;
	~BitCrusher() = default;

	inline void init(const int sampleRate, const int channels)
	{
		m_sampleRate = sampleRate;

		m_samplesToHold.resize(channels);
		m_holdValue.resize(channels);

		m_filter.resize(channels);
		for (int i = 0; i < m_filter.size(); i++)
		{
			m_filter[i].init(sampleRate);
		}
	}
	inline void set(const float bitDepth, const int downSample, const float mix, const float frequency)
	{
		m_bitDepth = bitDepth;
		m_downSample = downSample;
		m_mix = mix;

		m_quantizationLevels = std::powf(2.0f, m_bitDepth);
		m_quantizationStep = 1.0f / m_quantizationLevels;

		for (int i = 0; i < m_filter.size(); i++)
		{
			m_filter[i].set(frequency);
		}
	}
	inline void processBlock(juce::AudioBuffer<float>& buffer)
	{
		const auto channels = buffer.getNumChannels();
		const auto samples = buffer.getNumSamples();

		for (int channel = 0; channel < channels; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);

			// References
			auto& samplesToHold = m_samplesToHold[channel];
			auto& holdValue = m_holdValue[channel];
			auto& filter = m_filter[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				// Read
				const float in = channelBuffer[sample];

				//BitCrush
				const float quantizated = static_cast<int>(in * m_quantizationLevels) * m_quantizationStep;

				//Filter
				const float filtered = filter.process(quantizated);

				// Downsample
				samplesToHold--;
				if (samplesToHold <= 0)
				{				
					samplesToHold = m_downSample;				
					holdValue = filtered;
				}

				// Wet/Dry mix
				channelBuffer[sample] = in + m_mix * (holdValue - in);
			}
		}
	}
	inline void release()
	{
		for (int i = 0; i < m_filter.size(); i++)
		{
			m_filter[i].release();
		}
		m_filter.clear();
		m_samplesToHold.clear();
		m_holdValue.clear();
		m_bitDepth = 8.0f;
		m_mix = 1.0f;
		m_quantizationLevels = 128.0f;
		m_quantizationStep = 0.0078125f;
		m_downSample = 1;
		m_sampleRate = 48000;
	}

private:
	std::vector<SixthOrderLowPassFilter> m_filter;
	std::vector<float> m_holdValue;
	std::vector<int> m_samplesToHold;
	float m_bitDepth = 8.0f;
	float m_mix = 1.0f;
	float m_quantizationLevels = 128.0f;
	float m_quantizationStep = 0.0078125f;
	int m_downSample = 1;
	int m_sampleRate = 48000;
};