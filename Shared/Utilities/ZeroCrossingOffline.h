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

	enum Type
	{
		Default,
		Filter,
		Frequency	// Limit minimum distance between zero crossings based on frequency
	};

	void init(const int sampleRate)
	{
		m_filter.init(sampleRate);
		m_sampleRate = sampleRate;
	}
	void set(const float frequency, const int searchRange)
	{
		m_filter.setBandPassPeakGain(frequency, 2.0f);
		m_frequency = frequency;
		m_searchRange = searchRange;
	}
	void setType(const int type)
	{
		m_type = static_cast<Type>(type - 1);
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
		
		std::vector<int> zeroCrossingEstimatedIdx;
		auto* bufferSum = sumAudioBuffer.getWritePointer(0);

		if (m_type == Type::Default)
		{
			zeroCrossingEstimatedIdx.push_back(0);

			// Dont allow zero crosing too often
			int sinceLast = 100;

			const float threshold = juce::Decibels::decibelsToGain(-20.0f);

			bool wasPositive = false;
			bool wasNegative = false;

			float inLast = bufferSum[0];

			for (int sample = 1; sample < samples; sample++)
			{
				const float in = bufferSum[sample];

				if (in > threshold)
				{
					wasPositive = true;
				}

				if (in < -threshold)
				{
					wasNegative = true;
				}

				if (inLast < 0.0f && in >= 0.0f && sinceLast > 100 && wasPositive && wasNegative)
				{
					// Choose sample closer to 0
					if (std::fabsf(inLast) > std::fabsf(in))
					{
						zeroCrossingEstimatedIdx.push_back(sample);
					}
					else
					{
						zeroCrossingEstimatedIdx.push_back(sample - 1);
					}
										
					wasPositive = false;
					wasNegative = false;
					sinceLast = 0;
				}

				inLast = in;
				sinceLast++;
			}

			zeroCrossingEstimatedIdx.push_back(samples);
		}
		else if (m_type == Type::Filter)
		{
			zeroCrossingEstimatedIdx.push_back(0);

			float inLast = m_filter.processDF1(bufferSum[0]);

			for (int sample = 1; sample < samples; sample++)
			{
				float in = bufferSum[sample];
				in = m_filter.processDF1(in);

				if (inLast < 0.0f && in >= 0.0f)
				{
					zeroCrossingEstimatedIdx.push_back(sample);
				}

				inLast = in;
			}

			zeroCrossingEstimatedIdx.push_back(samples);
		}
		else if (m_type == Type::Frequency)
		{
			// Calculate minimum zero crossing distance
			const int minDistance = (int)(0.8f * (float)m_sampleRate / m_frequency);

			zeroCrossingEstimatedIdx.push_back(0);

			float inLast = m_filter.processDF1(bufferSum[0]);

			for (int sample = 1; sample < samples; sample++)
			{
				float in = bufferSum[sample];
				in = m_filter.processDF1(in);

				if (inLast < 0.0f && in >= 0.0f && sample - zeroCrossingEstimatedIdx.back() > minDistance)
				{
					zeroCrossingEstimatedIdx.push_back(sample);
				}

				inLast = in;
			}

			zeroCrossingEstimatedIdx.push_back(samples);
		}

		// Remove false positive first and last
		const int testRangeHalf = m_searchRange / 2;

		if (zeroCrossingEstimatedIdx[1] - testRangeHalf <= 0)
		{
			zeroCrossingEstimatedIdx.erase(zeroCrossingEstimatedIdx.begin() + 1);
		}

		if (zeroCrossingEstimatedIdx[zeroCrossingEstimatedIdx.size() - 2] + testRangeHalf >= samples)
		{
			zeroCrossingEstimatedIdx.erase(zeroCrossingEstimatedIdx.end() - 1);
		}
		
		//regions = zeroCrossingEstimatedIdx;

		// Final zero corssing found in unfiltered signal
		regions.resize(zeroCrossingEstimatedIdx.size());

		regions[0] = 0;

		for (int segmentId = 1; segmentId < regions.size() - 1; segmentId++)
		{			
			const int sampleStart = zeroCrossingEstimatedIdx[segmentId] - testRangeHalf;		
			const int sampleEnd = zeroCrossingEstimatedIdx[segmentId] + testRangeHalf;

			float inLast = bufferSum[sampleStart - 1];
			int closestIndex = sampleStart;

			for (int sample = sampleStart; sample < sampleEnd; sample++)
			{
				const float in = bufferSum[sample];

				if (inLast < 0.0f && in >= 0.0f)
				{
					// Find closes sample to estimated zero crossing
					if (std::abs(zeroCrossingEstimatedIdx[segmentId] - sample) < std::abs(zeroCrossingEstimatedIdx[segmentId] - closestIndex))
					{
						closestIndex = sample;
					}
				}

				inLast = in;
			}

			regions[segmentId] = closestIndex;
		}

		regions[regions.size() - 1] = samples;

		//Debug
		/*std::vector<int> diff;
		diff.resize(zeroCrossingEstimatedIdx.size());

		for (int segmentId = 0; segmentId < regions.size(); segmentId++)
		{
			diff[segmentId] = zeroCrossingEstimatedIdx[segmentId] - regions[segmentId];
		}*/
	}

private:
	BiquadFilter m_filter;
	float m_frequency = 20.0f;
	int m_sampleRate = 48000;
	int m_searchRange = 100;
	Type m_type = Type::Default;
};
