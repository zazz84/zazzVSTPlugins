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
		DominantFrequency
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
	void setFFTPhaseThreshold(const float thresholdDegrees)
	{
		// Convert degrees to radians: threshold is in range [-180, 180]
		m_fftPhaseThresholdRadians = thresholdDegrees * 3.14159265359f / 180.0f;
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

		// Handle final zero crossing
		regions.resize(zeroCrossingEstimatedIdx.size());

		if (m_type == Type::Default)
		{
			regions = zeroCrossingEstimatedIdx;
		}
		else if (m_type == Type::Filter)
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
		}
		else if (m_type == Type::DominantFrequency)
		{
			processDominantFrequencyDetection(sumAudioBuffer, regions);
		}
	}

private:
	void processDominantFrequencyDetection(const juce::AudioBuffer<float>& audioBuffer, std::vector<int>& regions)
	{
		regions.clear();

		const int samples = audioBuffer.getNumSamples();
		if (samples == 0)
		{
			return;
		}

		auto* bufferData = audioBuffer.getReadPointer(0);

		// FFT setup (same as SpectrogramDisplayComponent)
		constexpr int FFT_ORDER = 12;
		constexpr int FFT_SIZE = 1 << FFT_ORDER;
		constexpr float MIN_FREQUENCY = 20.0f;
		constexpr float MAX_FREQUENCY = 200.0f;
		constexpr int NUM_FREQUENCY_BINS = 128;
		constexpr int NUM_TIME_BINS = 2048;

		juce::dsp::FFT forwardFFT(FFT_ORDER);
		juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);

		const float binFrequencyResolution = (float)m_sampleRate / FFT_SIZE;
		const int hopSize = std::max(1, (samples - FFT_SIZE) / (NUM_TIME_BINS - 1));

		// Compute dominant frequency bins and phase information
		std::vector<float> phaseTrajectory;
		std::vector<int> frameStartSamples;
		std::vector<int> frameDominantFreqBins;
		int validTimeSteps = 0;

		for (int timeIdx = 0; timeIdx < NUM_TIME_BINS; ++timeIdx)
		{
			int startSample = timeIdx * hopSize;

			if (startSample + FFT_SIZE > samples)
			{
				break;
			}

			float fftData[2 * FFT_SIZE]{};

			// Copy audio data
			for (int i = 0; i < FFT_SIZE; ++i)
			{
				fftData[i] = bufferData[startSample + i];
			}

			// Apply windowing
			window.multiplyWithWindowingTable(fftData, FFT_SIZE);

			// Perform FFT - need full complex output for phase extraction
			forwardFFT.performRealOnlyForwardTransform(fftData);

			// Map FFT bins to frequency range [20Hz, 200Hz]
			const int minBin = (int)(MIN_FREQUENCY / binFrequencyResolution);
			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);
			const int freqBinRange = maxBin - minBin;

			int maxFreqIdx = 0;
			int maxFftBin = minBin;
			float maxMagnitudeFrame = 0.0f;

			for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
			{
				const int fftBin = minBin + (freqIdx * freqBinRange) / NUM_FREQUENCY_BINS;

				if (fftBin >= 0 && fftBin < FFT_SIZE)
				{
					const float real = fftData[2 * fftBin];
					const float imag = fftData[2 * fftBin + 1];
					const float magnitude = std::sqrt(real * real + imag * imag);

					if (magnitude > maxMagnitudeFrame)
					{
						maxMagnitudeFrame = magnitude;
						maxFreqIdx = freqIdx;
						maxFftBin = fftBin;
					}
				}
			}

			frameStartSamples.push_back(startSample);
			frameDominantFreqBins.push_back(maxFftBin);

			// Extract phase from dominant frequency bin
			const float real = fftData[2 * maxFftBin];
			const float imag = fftData[2 * maxFftBin + 1];
			const float phase = std::atan2(imag, real);
			phaseTrajectory.push_back(phase);

			validTimeSteps++;
		}

		if (phaseTrajectory.empty() || validTimeSteps == 0)
		{
			regions.push_back(0);
			return;
		}

		// Detect upward zero crossings using interpolated phase per sample
		regions.push_back(0);

		int sinceLast = 0;
		const int SINCE_LAST_MIN = (int)((float)m_sampleRate / m_maximumFrequency);
		const float PI = 3.14159265359f;

		// Helper lambda to wrap phase to [-π, π]
		auto wrapPhase = [PI](float phase) -> float {
			while (phase > PI)
				phase -= 2.0f * PI;
			while (phase < -PI)
				phase += 2.0f * PI;
			return phase;
		};

		float lastPhase = wrapPhase(phaseTrajectory[0]);

		for (int sample = 1; sample < samples; ++sample)
		{
			// Find which FFT frames this sample is between
			int frameIdx = std::min((sample - frameStartSamples[0]) / hopSize, (int)phaseTrajectory.size() - 1);
			frameIdx = std::max(0, frameIdx);

			// Interpolate phase between frames
			float currentPhase = phaseTrajectory[frameIdx];

			if (frameIdx + 1 < (int)phaseTrajectory.size())
			{
				// Linear interpolation between current and next frame
				int nextFrameStart = frameStartSamples[frameIdx + 1];
				int currentFrameStart = frameStartSamples[frameIdx];
				int frameDuration = nextFrameStart - currentFrameStart;

				if (frameDuration > 0)
				{
					float samplePositionInFrame = (float)(sample - currentFrameStart) / frameDuration;
					samplePositionInFrame = std::max(0.0f, std::min(1.0f, samplePositionInFrame));

					float nextPhase = phaseTrajectory[frameIdx + 1];

					// Interpolate accounting for phase wrapping
					float phaseDiff = nextPhase - currentPhase;
					if (phaseDiff > PI)
						phaseDiff -= 2.0f * PI;
					else if (phaseDiff < -PI)
						phaseDiff += 2.0f * PI;

					currentPhase = currentPhase + samplePositionInFrame * phaseDiff;
				}
			}

			// Wrap phase to [-π, π]
			currentPhase = wrapPhase(currentPhase);

			// Detect upward zero crossing: phase crosses threshold from below to above
			if (lastPhase < m_fftPhaseThresholdRadians && currentPhase >= m_fftPhaseThresholdRadians && sinceLast > SINCE_LAST_MIN)
			{
				regions.push_back(sample);
				sinceLast = 0;
			}
			else
			{
				sinceLast++;
			}

			lastPhase = currentPhase;
		}

		// Ensure we have at least beginning marker
		if (regions.empty())
		{
			regions.push_back(0);
		}
	}
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
	float m_fftPhaseThresholdRadians = 0.0f; // Default to 0 degrees (crossing through 0)
};
