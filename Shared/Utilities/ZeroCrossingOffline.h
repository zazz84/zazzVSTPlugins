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
	};

	void init(const int sampleRate)
	{
		m_filter.init(sampleRate);
		m_sampleRate = sampleRate;
	}
	void set(const float frequency, const int searchRange, const float threshold, const float maximumFrequency)
	{
		//m_filter.setBandPassPeakGain(frequency, 2.0f);
		m_filter.setLowPass(frequency, 4.0f);
		m_frequency = frequency;
		m_searchRange = searchRange;
		m_threshold = threshold;
		m_maximumFrequency = maximumFrequency;
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

		sumAudioBuffer.copyFrom(0, 0, audioBuffer.getReadPointer(0), audioBuffer.getNumSamples());

		/*if (channels == 1)
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
		}*/

		// Estimated zero crossing extracted from filtered signal
		m_filter.reset();
		
		std::vector<int> zeroCrossingEstimatedIdx;
		auto* bufferSum = sumAudioBuffer.getWritePointer(0);
		
		// Dont allow zero crosing too often
		int sinceLast = 0;
		const int SINCE_LAST_MIN = (int)((float)m_sampleRate / m_maximumFrequency);
			
		bool wasPositive = false;
		bool wasNegative = false;
	
		zeroCrossingEstimatedIdx.push_back(0);
		float inLast = bufferSum[0];

		for (int sample = 1; sample < samples; sample++)
		{
			float in = bufferSum[sample];

			if (m_type == Type::Filter)
			{
				in = m_filter.processDF1(in);
			}

			if (in > m_threshold)
			{
				wasPositive = true;
			}

			if (in < -m_threshold)
			{
				wasNegative = true;
			}

			if (inLast < 0.0f && in >= 0.0f && sinceLast > SINCE_LAST_MIN && wasPositive && wasNegative)
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

		// Remove false positive first and last
		const int testRangeHalf = m_searchRange / 2;

		/*if (zeroCrossingEstimatedIdx.size() > 1)
		{
			if (zeroCrossingEstimatedIdx[1] - testRangeHalf <= 0)
			{
				zeroCrossingEstimatedIdx.erase(zeroCrossingEstimatedIdx.begin() + 1);
			}
		}

		if (zeroCrossingEstimatedIdx.size() > 1)
		{
			if (zeroCrossingEstimatedIdx[zeroCrossingEstimatedIdx.size() - 1] + testRangeHalf >= samples)
			{
				zeroCrossingEstimatedIdx.erase(zeroCrossingEstimatedIdx.end());
			}
		}*/

		// Handle final zero crossing
		regions.resize(zeroCrossingEstimatedIdx.size());

		if (m_type == Type::Default)
		{
			regions = zeroCrossingEstimatedIdx;
		}
		else
		{
			// Final zero corssing with smaller amplitude
			regions[0] = 0;

			for (int segmentId = 1; segmentId < regions.size(); segmentId++)
			{
				const int sampleStart = zeroCrossingEstimatedIdx[segmentId] - testRangeHalf;
				const int sampleEnd = zeroCrossingEstimatedIdx[segmentId] + testRangeHalf;

				float inLast2 = bufferSum[sampleStart - 1];
				int closestIndex = sampleStart;

				for (int sample = sampleStart; sample < sampleEnd; sample++)
				{
					const float in = bufferSum[sample];

					if (inLast2 < 0.0f && in >= 0.0f)
					{
						// Find closes sample to estimated zero crossing
						if (std::abs(zeroCrossingEstimatedIdx[segmentId] - sample) < std::abs(zeroCrossingEstimatedIdx[segmentId] - closestIndex))
						{
							closestIndex = sample;
						}
					}

					inLast2 = in;
				}

				regions[segmentId] = closestIndex;
			}

			//regions[regions.size() - 1] = samples;

			// Try to find better zero crossings by comparing region length to its median
			/*std::vector<int> diff;
			diff.resize(regions.size() - 1);

			for (int segmentId = 0; segmentId < regions.size() - 1; segmentId++)
			{
				diff[segmentId] = regions[segmentId + 1] - regions[segmentId];
			}

			const int median = getMedian(diff);

			for (int i = 0; i < regions.size() - 2; i++)
			{
				const int currentCrossing = regions[i + 1];
				const int size = currentCrossing - regions[i];
				const int idealZeroCrossing = regions[i] + median;
				const int diff2 = std::abs(median - size);

				int indexLeft = idealZeroCrossing - diff2;
				for (int j = idealZeroCrossing - diff2; j < idealZeroCrossing; j++)
				{
					if (bufferSum[j - 1] < 0.0f && bufferSum[j] >= 0.0f)
					{
						indexLeft = j;
					}
				}

				int indexRight = idealZeroCrossing + diff2;
				for (int j = idealZeroCrossing + diff2; j >= idealZeroCrossing; j--)
				{
					if (bufferSum[j - 1] < 0.0f && bufferSum[j] >= 0.0f)
					{
						indexRight = j;
					}
				}

				const int betterIndex = idealZeroCrossing - indexLeft < indexRight - idealZeroCrossing ? indexLeft : indexRight;
				const int diffBetter = std::abs(idealZeroCrossing - betterIndex);

				if (diffBetter < diff2)
				{
					regions[i + 1] = betterIndex;
				}
			}*/
		}
	}

private:
	int getMedian(std::vector<int> input)
	{
		if (input.empty())
		{
			return 0;
		}

		std::sort(input.begin(), input.end());
		size_t n = input.size();

		if (n % 2 == 1) // odd number of elements
			return input[n / 2];
		else // even number of elements
			return (input[n / 2 - 1] + input[n / 2]) / 2;
	}

	BiquadFilter m_filter;
	float m_frequency = 20.0f;
	float m_maximumFrequency = 200.0f;
	float m_threshold = -60.0f;
	int m_sampleRate = 48000;
	int m_searchRange = 100;
	Type m_type = Type::Default;
};
