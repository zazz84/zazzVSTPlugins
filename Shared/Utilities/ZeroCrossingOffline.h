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
		if (m_type == Type::Default || m_type == Type::Filter)
		{
			processTimeDomainDetection(sumBuffer, regions);
		}
		else if (m_type == Type::DominantFrequency)
		{
			processDominantFrequencyDetection(sumBuffer, regions);
		}
	}

private:
	void processTimeDomainDetection(const juce::AudioBuffer<float>& buffer, std::vector<int>& regions)
	{
		const auto samples = buffer.getNumSamples();
		auto* pBuffer = buffer.getReadPointer(0);

		std::vector<int> zeroCrossingIdx;

		// Dont allow zero crosing too often
		int sinceLast = 0;
		const int SINCE_LAST_MIN = (int)((float)m_sampleRate / m_maximumFrequency);

		bool wasPositive = false;
		bool wasNegative = false;

		zeroCrossingIdx.push_back(0);
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
					zeroCrossingIdx.push_back(sample);
				}
				else
				{
					zeroCrossingIdx.push_back(sample - 1);
				}

				wasPositive = false;
				wasNegative = false;
				sinceLast = 0;
			}

			inLast = in;
			sinceLast++;
		}

		// Output results
		regions = zeroCrossingIdx;
	}


	void processDominantFrequencyDetection(const juce::AudioBuffer<float>& buffer, std::vector<int>& regions)
	{
		const auto samples = buffer.getNumSamples();
		auto* pBuffer = buffer.getReadPointer(0);

		// FFT setup (same as SpectrogramDisplayComponent)
		constexpr int FFT_ORDER = 14;
		constexpr int FFT_SIZE = 1 << FFT_ORDER;
		constexpr float MIN_FREQUENCY =48000.0f / (float)FFT_SIZE; // = Bins size in Hz, so 48000/4096 = ~11.7Hz
		constexpr int NUM_FREQUENCY_BINS = 128;
		constexpr float MAX_FREQUENCY = MIN_FREQUENCY * (float)NUM_FREQUENCY_BINS;
		const int NUM_TIME_BINS = std::max(1, (samples * 2048) / m_sampleRate);

		juce::dsp::FFT forwardFFT(FFT_ORDER);
		juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);

		const float binFrequencyResolution = (float)m_sampleRate / FFT_SIZE;

		// Compute dominant frequency bins and phase information
		std::vector<float> phaseTrajectory;
		std::vector<int> frameStartSamples;
		std::vector<int> frameDominantFreqBins;
		int validTimeSteps = 0;

		// Divide buffer into NUM_TIME_BINS equal blocks
		const float blockSize = (float)samples / NUM_TIME_BINS;

		for (int timeIdx = 0; timeIdx < NUM_TIME_BINS; ++timeIdx)
		{
			// Calculate center sample of this block
			float centerSampleFloat = blockSize * (timeIdx + 0.5f);
			int centerSample = (int)centerSampleFloat;

			// Start of FFT window (centered at centerSample)
			int startSample = centerSample - FFT_SIZE / 2;

			float fftData[2 * FFT_SIZE]{};

			// Copy audio data with circular wrapping
			for (int i = 0; i < FFT_SIZE; ++i)
			{
				int sampleIdx = (startSample + i) % samples;
				// Handle negative modulo for C++
				if (sampleIdx < 0)
					sampleIdx += samples;
				fftData[i] = pBuffer[sampleIdx];
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

			frameStartSamples.push_back(centerSample);
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

	float m_maximumFrequency = 200.0f;
	float m_threshold = -60.0f;
	int m_sampleRate = 48000;
	Type m_type = Type::Default;
	float m_fftPhaseThresholdRadians = 0.0f; // Default to 0 degrees (crossing through 0)
};