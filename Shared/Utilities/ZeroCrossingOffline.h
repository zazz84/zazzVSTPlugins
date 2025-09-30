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

#include <vector>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class ZeroCrossingOffline
{
public:
	ZeroCrossingOffline() = default;
	~ZeroCrossingOffline() = default;

	static const int N_CHANNELS = 2;

	void init(const int sampleRate)
	{
		m_filter.init(sampleRate);
	}
	void set(const float frequency, const int searchRange)
	{
		m_filter.setBandPassPeakGain(frequency, 0.707f);
		m_searchRange = searchRange;
	}
	void process(const juce::AudioBuffer<float>& audioBuffer, std::vector<int>& regions)
	{
		regions.clear();

		const auto samples = audioBuffer.getNumSamples();
		const auto channels = std::min(audioBuffer.getNumChannels(), N_CHANNELS);

		// Create sum buffer to handle stereo input
		juce::AudioBuffer<float> sumAudioBuffer(1, samples);

		if (channels == 1)
		{
			sumAudioBuffer.makeCopyOf(audioBuffer, true);
		}
		else
		{
			auto* bufferLeft = audioBuffer.getReadPointer(0);
			auto* bufferRight = audioBuffer.getReadPointer(1);
			auto* bufferSum = sumAudioBuffer.getWritePointer(0);

			for (int sample = 0; sample < samples; sample++)
			{
				bufferSum[sample] = 0.5f * (bufferLeft[sample] + bufferRight[sample]);
			}
		}

		// Estimated zero crossing extracted from filtered signal
		m_filter.reset();
		
		std::vector<int> m_zeroCrossingEstimatedIdx;

		m_zeroCrossingEstimatedIdx.push_back(0);

		auto* bufferSum = sumAudioBuffer.getWritePointer(0);

		float inLast = m_filter.processDF1(bufferSum[0]);

		for (int sample = 1; sample < samples; sample++)
		{
			float in = bufferSum[sample];
			in = m_filter.processDF1(in);

			if (inLast < 0.0f && in >= 0.0f)
			{
				m_zeroCrossingEstimatedIdx.push_back(sample);
			}

			inLast = in;
		}

		m_zeroCrossingEstimatedIdx.push_back(samples);

		// Final zero corssing found in unfiltered signal
		regions.resize(m_zeroCrossingEstimatedIdx.size());

		regions[0] = 0;

		for (int segmentId = 1; segmentId < regions.size() - 1; segmentId++)
		{
			const int testRangeHalf = m_searchRange / 2;
			const int sampleStart = std::max(0, m_zeroCrossingEstimatedIdx[segmentId] - testRangeHalf);
			const int sampleEnd = m_zeroCrossingEstimatedIdx[segmentId] + testRangeHalf;

			float inLast = bufferSum[sampleStart - 1];
			int closestIndex = sampleStart;

			for (int sample = sampleStart; sample < sampleEnd; sample++)
			{
				const float in = bufferSum[sample];

				if (inLast < 0.0f && in >= 0.0f)
				{
					if (std::abs(m_zeroCrossingEstimatedIdx[segmentId] - sample) < std::abs(m_zeroCrossingEstimatedIdx[segmentId] - closestIndex))
					{
						closestIndex = sample;
					}
				}

				inLast = in;
			}

			regions[segmentId] = closestIndex;
		}

		regions[regions.size() - 1] = samples;

		// Debug
		std::vector<int> m_zeroCrossingDiff;
		m_zeroCrossingDiff.resize(regions.size());

		for (int segmentId = 0; segmentId < regions.size(); segmentId++)
		{
			m_zeroCrossingDiff[segmentId] = regions[segmentId] - m_zeroCrossingEstimatedIdx[segmentId];
		}
	}

private:
	BiquadFilter m_filter;
	int m_searchRange = 100;
};
