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
#include "../../zazzDSP/Utilities/Spectrum.h"
#include "../Filters/BiquadFilters.h"

#include <vector>
#include <algorithm>
#include <cmath>

class ZeroCrossingOffline
{
public:
	ZeroCrossingOffline() = default;
	~ZeroCrossingOffline() = default;

	static const int N_CHANNELS = 2;
	// FFT setup
	static const int FFT_ORDER = 12;
	static const int FFT_SIZE = 1 << FFT_ORDER;

	enum Type
	{
		Default,
		Filter,
		DominantFrequency,
		FFTFilter
	};

	void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	}
	void set(const float threshold, const float maximumFrequency)
	{
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
	void setMinimumLengthMultiplier(const float multiplier)
	{
		m_minimumLengthMultiplier = multiplier;
	}
	const std::vector<float>& getLastFilteredBuffer() const
	{
		return m_lastFilteredBuffer;
	}
	void process(const juce::AudioBuffer<float>& audioBuffer, std::vector<int>& regions)
	{
		regions.clear();

		const auto samples = audioBuffer.getNumSamples();
		if (samples == 0)
		{
			return;
		}

		// Create temporary sum buffer
		// TODO - Handle multichannel buffers better
		juce::AudioBuffer<float> sumBuffer(1, samples);
		sumBuffer.copyFrom(0, 0, audioBuffer.getReadPointer(0), samples);

		// Delegate to appropriate detection method based on type
		// Fallback to time-domain detection for small buffers or non-FFT types to avoid overhead
		if (m_type == Type::Default || m_type == Type::Filter || samples < FFT_SIZE)
		{
			processTimeDomainDetection(sumBuffer, regions);
		}
		else if (m_type == Type::DominantFrequency)
		{
			processDominantFrequencyDetection(sumBuffer, regions);
		}
		else if (m_type == Type::FFTFilter)
		{
			processFFTFilterDetection(sumBuffer, regions);
		}
	}

private:
	void processTimeDomainDetection(const juce::AudioBuffer<float>& buffer, std::vector<int>& regions)
	{
		const auto samples = buffer.getNumSamples();
		auto* pBuffer = buffer.getReadPointer(0);

		// Dont allow zero crosing too often
		int sinceLast = 0;
		const int SINCE_LAST_MIN = (int)((float)m_sampleRate / m_maximumFrequency);

		bool wasPositive = false;
		bool wasNegative = false;

		regions.push_back(0);
		float inLast = pBuffer[0];

		for (int sample = 1; sample < samples; sample++)
		{
			float in = pBuffer[sample];

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
					regions.push_back(sample);
				}
				else
				{
					regions.push_back(sample - 1);
				}

				wasPositive = false;
				wasNegative = false;
				sinceLast = 0;
			}

			inLast = in;
			sinceLast++;
		}
	}


	void processDominantFrequencyDetection(const juce::AudioBuffer<float>& buffer, std::vector<int>& regions)
	{
		const auto samples = buffer.getNumSamples();

		// Calculate dominant frequencies and phase trajectory using extended method
		std::vector<float> phaseTrajectory;
		std::vector<int> frameStartSamples;
		std::vector<float> dominantFrequencies;
		zazzDSP::Spectrum::calculateDominantFrequencies(buffer, m_sampleRate, dominantFrequencies, frameStartSamples, &phaseTrajectory, true);

		// Calculate block size for phase interpolation
		const int NUM_TIME_BINS = zazzDSP::Spectrum::calculateNumTimeBins(samples, m_sampleRate);
		const float blockSize = (float)samples / NUM_TIME_BINS;

		if (phaseTrajectory.empty() || phaseTrajectory.size() == 0)
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
			float sampleBlockPosition = (float)sample / blockSize;
			int frameIdx = (int)sampleBlockPosition;
			frameIdx = std::max(0, std::min(frameIdx, (int)phaseTrajectory.size() - 1));

			// Interpolate phase between frames
			float currentPhase = phaseTrajectory[frameIdx];

			if (frameIdx + 1 < (int)phaseTrajectory.size())
			{
				// Linear interpolation between current and next frame
				float currentFrameCenter = frameStartSamples[frameIdx];
				float nextFrameCenter = frameStartSamples[frameIdx + 1];
				float frameDuration = nextFrameCenter - currentFrameCenter;

				if (frameDuration > 0.0f)
				{
					float samplePositionInFrame = (float)(sample - currentFrameCenter) / frameDuration;
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

	void processFFTFilterDetection(const juce::AudioBuffer<float>& buffer, std::vector<int>& regions)
	{
		const auto samples = buffer.getNumSamples();
		auto* pBuffer = buffer.getReadPointer(0);

		// FFT setup (match SpectrogramDisplayComponent)
		constexpr int FFT_ORDER = 12;
		constexpr int FFT_SIZE = 1 << FFT_ORDER;
		constexpr int BINS_PER_SECOND = 512;

		const int NUM_TIME_BINS = std::max(1, (samples * BINS_PER_SECOND) / m_sampleRate);
		const float blockSize = (float)samples / NUM_TIME_BINS;

		// Step 1: Calculate initial dominant frequencies from original buffer (for filtering)
		std::vector<float> dominantFrequencies;
		std::vector<int> frameCenterSamples;
		zazzDSP::Spectrum::calculateDominantFrequencies(buffer, m_sampleRate, dominantFrequencies, frameCenterSamples);

		// Step 2: Initialize filtered buffer with original data
		m_lastFilteredBuffer.resize(samples);
		std::copy(pBuffer, pBuffer + samples, m_lastFilteredBuffer.begin());

		// Forward pass: Apply bandpass filter using dominant frequency
		applyAdaptiveFilter(m_lastFilteredBuffer.data(), samples, dominantFrequencies, frameCenterSamples);

		// Step 3: Reverse the filtered buffer
		std::reverse(m_lastFilteredBuffer.begin(), m_lastFilteredBuffer.end());

		// Step 4: Reverse the dominant frequencies array and frame samples for backward pass
		std::vector<float> reversedDominantFrequencies = dominantFrequencies;
		std::reverse(reversedDominantFrequencies.begin(), reversedDominantFrequencies.end());

		// Reverse the frame center samples and adjust them
		std::vector<int> reversedFrameCenterSamples = frameCenterSamples;
		for (int i = 0; i < (int)reversedFrameCenterSamples.size(); ++i)
		{
			reversedFrameCenterSamples[i] = samples - 1 - reversedFrameCenterSamples[i];
		}
		std::reverse(reversedFrameCenterSamples.begin(), reversedFrameCenterSamples.end());

		// Step 5: Backward pass: Apply filter again (reversed)
		applyAdaptiveFilter(m_lastFilteredBuffer.data(), samples, reversedDominantFrequencies, reversedFrameCenterSamples);

		// Step 6: Reverse the buffer back
		std::reverse(m_lastFilteredBuffer.begin(), m_lastFilteredBuffer.end());

		// Step 7: Detect zero crossings on filtered audio using dominant frequencies from initial analysis
		int sinceLast = 0;

		bool wasPositive = false;
		bool wasNegative = false;

		regions.push_back(0);
		float inLast = m_lastFilteredBuffer[0];

		for (int sample = 1; sample < samples; ++sample)
		{
			float in = m_lastFilteredBuffer[sample];

			if (in > m_threshold)
			{
				wasPositive = true;
			}

			if (in < -m_threshold)
			{
				wasNegative = true;
			}

			// Calculate minimum length dynamically based on dominant frequency at this sample
			int SINCE_LAST_MIN = (int)((float)m_sampleRate / m_maximumFrequency);

			// Always use dominant frequency from initial analysis, scaled by multiplier
			if (!dominantFrequencies.empty())
			{
				int timeIdx = (int)((float)sample / blockSize);
				timeIdx = std::min(timeIdx, (int)dominantFrequencies.size() - 1);
				
				if (timeIdx >= 0)
				{
					float domFreq = dominantFrequencies[timeIdx];
					if (domFreq > 0.0f)
					{
						float dynamicMinLength = ((float)m_sampleRate / domFreq) * m_minimumLengthMultiplier;
						SINCE_LAST_MIN = (int)dynamicMinLength;
					}
				}
			}

			if (inLast < 0.0f && in >= 0.0f && sinceLast > SINCE_LAST_MIN && wasPositive && wasNegative)
			{
				// Choose sample closer to 0
				if (std::fabsf(inLast) > std::fabsf(in))
				{
					regions.push_back(sample);
				}
				else
				{
					regions.push_back(sample - 1);
				}

				wasPositive = false;
				wasNegative = false;
				sinceLast = 0;
			}

			inLast = in;
			sinceLast++;
		}
	}

	void applyAdaptiveFilter(float* buffer, int samples, const std::vector<float>& dominantFrequencies, const std::vector<int>& frameCenterSamples)
	{
		if (samples == 0 || dominantFrequencies.empty() || frameCenterSamples.empty())
		{
			return;
		}

		// Create persistent filter objects that maintain state across all samples
		BiquadFilter filterHP, filterLP;
		filterHP.init(m_sampleRate);
		filterLP.init(m_sampleRate);

		const int frameDuration = samples / dominantFrequencies.size();

		// Process each sample with filters tuned to local dominant frequency
		for (int sample = 0; sample < samples; ++sample)
		{
			// Calculate which frame this sample belongs to based on uniform frame spacing
			float frameIdx = (float)(sample) / (float)frameDuration - 0.5f;

			// Interpolate dominant frequency between frames
			float centerFrequency = 0.0f;

			int idx0 = (int)std::floor(frameIdx);
			int idx1 = idx0 + 1;
			float alpha = frameIdx - idx0;
				
			if (idx0 < 0)
			{
				centerFrequency = dominantFrequencies[0];
			}
			if (idx0 >= 0 && idx1 < (int)dominantFrequencies.size() - 1)
			{
				centerFrequency = (1.0f - alpha) * dominantFrequencies[idx0] + alpha * dominantFrequencies[idx1];
			}
			else if (idx1 >= (int)dominantFrequencies.size() - 1)
			{
				centerFrequency = dominantFrequencies.back();
			}

			// Clamp frequency to reasonable range
			centerFrequency = std::max(20.0f, std::min(20000.0f, centerFrequency));

			// Set filter frequencies based on dominant frequency (with some scaling to create a band)
			//const float highPassFrequency = centerFrequency * 0.7f;
			//const float lowPassFrequency = centerFrequency * 1.3f;

			//filterHP.setHighPass(highPassFrequency, 2.0f);
			//filterLP.setLowPass(lowPassFrequency, 2.0f);

			filterHP.setBandPassPeakGain(centerFrequency, 2.0f);

			// Process sample through both filters (filters maintain state across iterations)
			float sample_data = buffer[sample];
			sample_data = filterHP.processDF1(sample_data);
			//sample_data = filterLP.processDF1(sample_data);

			buffer[sample] = sample_data;
		}
	}

	float m_maximumFrequency = 200.0f;
	float m_threshold = -60.0f;
	int m_sampleRate = 48000;
	Type m_type = Type::Default;
	float m_fftPhaseThresholdRadians = 0.0f; // Default to 0 degrees (crossing through 0)
	float m_minimumLengthMultiplier = 1.0f; // Multiplier for minimum region length in FFT + Filter mode
	std::vector<float> m_lastFilteredBuffer; // Stores filtered buffer for visualization in FFT + Filter mode
};