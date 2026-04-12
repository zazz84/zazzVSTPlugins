#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace zazzDSP
{
	class Spectrum
	{
	public:
		static const int BINS_PER_SECOND = 512;

	private:
		/// <summary>
		/// Interpolates phase values between two adjacent bins with proper phase wrapping.
		/// Phase is interpolated linearly in the shortest direction around the unit circle.
		/// </summary>
		/// <param name="phase1">Phase at first bin (radians)</param>
		/// <param name="phase2">Phase at second bin (radians)</param>
		/// <param name="t">Interpolation parameter (0 to 1, where 0 = phase1, 1 = phase2)</param>
		/// <returns>Interpolated phase in range [-π, π]</returns>
		static float interpolatePhaseValue(float phase1, float phase2, float t)
		{
			constexpr float TWO_PI = 2.0f * 3.14159265358979323846f;
			constexpr float PI = 3.14159265358979323846f;

			float diff = phase2 - phase1;

			// Unwrap to shortest path around the circle
			if (diff > PI)
				diff -= TWO_PI;
			else if (diff < -PI)
				diff += TWO_PI;

			float result = phase1 + t * diff;

			// Wrap result back to [-π, π]
			while (result > PI)
				result -= TWO_PI;
			while (result < -PI)
				result += TWO_PI;

			return result;
		}

	public:

		/// <summary>
		/// Calculates FFT magnitude data for all time bins using centered windows and real-only FFT.
		/// Used for detailed frequency analysis including phase extraction.
		/// </summary>
		/// <param name="buffer">Input audio buffer (single channel)</param>
		/// <param name="sampleRate">Sample rate of the audio buffer in Hz</param>
		/// <param name="fftOrder">FFT order (FFT_SIZE = 2^fftOrder). Default is 12 (4096 samples)</param>
		/// <param name="outMagnitudes">Output 2D vector of FFT magnitudes [timeIdx][freqBin]</param>
		/// <param name="outPhaseData">Output 2D vector of FFT phases [timeIdx][freqBin] (optional)</param>
		/// <param name="outFrameCenterSamples">Output vector of center sample indices for each time bin</param>
		/// <param name="outBinFrequencyResolution">Output frequency resolution in Hz per FFT bin</param>
		/// <param name="outNumTimeBins">Output number of time bins calculated</param>
		/// <param name="extractPhase">If true, extract phase information</param>
		/// <param name="binsPerSecond">Time-frequency resolution in bins per second (default 512)</param>
		static void calculateFFTMagnitudes(
			const juce::AudioBuffer<float>& buffer,
			int sampleRate,
			int fftOrder,
			std::vector<std::vector<float>>& outMagnitudes,
			std::vector<int>& outFrameCenterSamples,
			float& outBinFrequencyResolution,
			int& outNumTimeBins,
			std::vector<std::vector<float>>* outPhaseData = nullptr,
			bool extractPhase = false,
			int binsPerSecond = BINS_PER_SECOND)
		{
			const int FFT_SIZE = 1 << fftOrder;
			const auto samples = buffer.getNumSamples();
			auto* pBuffer = buffer.getReadPointer(0);

			// Input buffer is too small for FFT
			if (samples < FFT_SIZE)
			{
				outNumTimeBins = 0;
				return;
			}

			const int NUM_TIME_BINS = calculateNumTimeBins(samples, sampleRate, binsPerSecond);

			juce::dsp::FFT forwardFFT(fftOrder);
			juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);

			outBinFrequencyResolution = (float)sampleRate / FFT_SIZE;
			const float blockSize = (float)samples / NUM_TIME_BINS;

			outMagnitudes.assign(NUM_TIME_BINS, std::vector<float>(FFT_SIZE / 2 + 1, 0.0f));
			outFrameCenterSamples.assign(NUM_TIME_BINS, 0);
			if (outPhaseData != nullptr && extractPhase)
			{
				outPhaseData->assign(NUM_TIME_BINS, std::vector<float>(FFT_SIZE / 2 + 1, 0.0f));
			}

			std::vector<float> fftData(2 * FFT_SIZE);

			for (int timeIdx = 0; timeIdx < NUM_TIME_BINS; ++timeIdx)
			{
				float centerSampleFloat = blockSize * (timeIdx + 0.5f);
				int centerSample = (int)centerSampleFloat;
				int startSample = centerSample - FFT_SIZE / 2;

				if (startSample < 0)
				{
					startSample = 0;
				}
				else if (startSample + FFT_SIZE > samples)
				{
					startSample = samples - FFT_SIZE;
				}

				outFrameCenterSamples[timeIdx] = centerSample;

				// Clear FFT buffer
				std::fill(fftData.begin(), fftData.end(), 0.0f);

				// Copy audio data
				std::copy(pBuffer + startSample, pBuffer + startSample + FFT_SIZE, fftData.begin());

				// Apply windowing
				window.multiplyWithWindowingTable(fftData.data(), FFT_SIZE);

				// Perform FFT using real-only transform for phase extraction capability
				forwardFFT.performRealOnlyForwardTransform(fftData.data());

				// Extract magnitudes and optionally phases
				const int numBins = FFT_SIZE / 2 + 1;
				for (int bin = 0; bin < numBins; ++bin)
				{
					const float real = fftData[2 * bin];
					const float img = fftData[2 * bin + 1];
					const float magnitude = std::sqrt(real * real + img * img);
					outMagnitudes[timeIdx][bin] = magnitude;

					if (outPhaseData != nullptr && extractPhase)
					{
						const float phase = std::atan2(img, real);
						(*outPhaseData)[timeIdx][bin] = phase;
					}
				}
			}

			outNumTimeBins = NUM_TIME_BINS;
		}

		/// <summary>
		/// Finds dominant frequencies from pre-calculated FFT magnitude data.
		/// Searches within a specified frequency bin range and extracts dominant frequency for each frame.
		/// </summary>
		/// <param name="magnitudes">2D vector of FFT magnitudes [timeIdx][freqBin]</param>
		/// <param name="binFrequencyResolution">Frequency resolution in Hz per FFT bin</param>
		/// <param name="minBin">Minimum FFT bin index to search</param>
		/// <param name="maxBin">Maximum FFT bin index to search</param>
		/// <param name="outDominantFrequencies">Output vector of dominant frequencies in Hz for each frame</param>
		/// <param name="inPhaseData">Optional input phase data [timeIdx][freqBin]</param>
		/// <param name="outPhaseAtDominant">Optional output phase values at dominant frequencies</param>
		static void findDominantFrequenciesFromFFT(
			const std::vector<std::vector<float>>& magnitudes,
			float binFrequencyResolution,
			int minBin,
			int maxBin,
			std::vector<float>& outDominantFrequencies,
			const std::vector<std::vector<float>>* inPhaseData = nullptr,
			std::vector<float>* outPhaseAtDominant = nullptr)
		{
			const int NUM_TIME_BINS = magnitudes.size();
			outDominantFrequencies.assign(NUM_TIME_BINS, 0.0f);
			if (outPhaseAtDominant != nullptr)
			{
				outPhaseAtDominant->assign(NUM_TIME_BINS, 0.0f);
			}

			for (int timeIdx = 0; timeIdx < NUM_TIME_BINS; ++timeIdx)
			{
				const int maxBinSize = (int)magnitudes[timeIdx].size();
				int maxFftBin = minBin;
				float maxMagnitude = 0.0f;

				for (int bin = minBin; bin < maxBin && bin < maxBinSize; ++bin)
				{
					if (magnitudes[timeIdx][bin] > maxMagnitude)
					{
						maxMagnitude = magnitudes[timeIdx][bin];
						maxFftBin = bin;
					}
				}

				// Interpolate frequency based on surrounding bins using parabolic interpolation
				float binOffset = 0.0f;

				if (maxFftBin > minBin && maxFftBin < maxBin - 1 && maxFftBin + 1 < maxBinSize)
				{
					float leftMag = magnitudes[timeIdx][maxFftBin - 1];
					float rightMag = magnitudes[timeIdx][maxFftBin + 1];
					float denominator = leftMag - 2.0f * maxMagnitude + rightMag;

					// Parabolic interpolation: find the bin offset for the true peak
					if (std::abs(denominator) > 1e-6f)
					{
						binOffset = 0.5f * (leftMag - rightMag) / denominator;
						// Clamp offset to reasonable range to avoid outliers
						binOffset = std::max(-0.5f, std::min(0.5f, binOffset));
					}
				}

				// Convert FFT bin to frequency in Hz with interpolated offset
				outDominantFrequencies[timeIdx] = (maxFftBin + binOffset) * binFrequencyResolution;

				// Extract and interpolate phase if provided
				if (inPhaseData != nullptr && outPhaseAtDominant != nullptr && timeIdx < (int)inPhaseData->size())
				{
					const int phaseSize = (int)(*inPhaseData)[timeIdx].size();
					if (maxFftBin < phaseSize)
					{
						float interpolatedPhase = (*inPhaseData)[timeIdx][maxFftBin];

						// If we have a sub-bin offset and neighboring bins, interpolate the phase
						if (std::abs(binOffset) > 1e-6f)
						{
							if (binOffset > 0.0f && maxFftBin + 1 < phaseSize)
							{
								// Interpolate toward right neighbor
								float rightPhase = (*inPhaseData)[timeIdx][maxFftBin + 1];
								interpolatedPhase = interpolatePhaseValue(interpolatedPhase, rightPhase, binOffset);
							}
							else if (binOffset < 0.0f && maxFftBin > 0)
							{
								// Interpolate toward left neighbor
								float leftPhase = (*inPhaseData)[timeIdx][maxFftBin - 1];
								interpolatedPhase = interpolatePhaseValue(leftPhase, interpolatedPhase, -binOffset);
							}
						}

						(*outPhaseAtDominant)[timeIdx] = interpolatedPhase;
					}
				}
			}
		}

		/// <summary>
		/// Convenience method that combines FFT calculation and dominant frequency detection.
		/// Performs FFT on centered windows and detects dominant frequencies in specified range.
		/// </summary>
		/// <param name="buffer">Input audio buffer (single channel)</param>
		/// <param name="sampleRate">Sample rate of the audio buffer in Hz</param>
		/// <param name="dominantFrequencies">Output vector of dominant frequencies in Hz</param>
		/// <param name="timeBinCenterSamples">Output vector of center sample indices for each time bin</param>
		/// <param name="outPhaseTrajectory">Output vector of phase values at dominant frequencies (optional)</param>
		/// <param name="usePhaseExtraction">If true, extract and return phase information</param>
		/// <param name="binsPerSecond">Time-frequency resolution in bins per second (default 512)</param>
		static void calculateDominantFrequencies(
			const juce::AudioBuffer<float>& buffer,
			int sampleRate,
			std::vector<float>& dominantFrequencies,
			std::vector<int>& timeBinCenterSamples,
			std::vector<float>* outPhaseTrajectory = nullptr,
			bool usePhaseExtraction = false,
			int binsPerSecond = BINS_PER_SECOND)
		{
			constexpr int FFT_ORDER = 12;
			constexpr int MIN_BIN = 1;
			constexpr float MAX_FREQUENCY = 400.0f;

			std::vector<std::vector<float>> magnitudes;
			std::vector<std::vector<float>> phases;
			float binFrequencyResolution = 0.0f;
			int numTimeBins = 0;

			// Calculate FFT magnitudes and optionally phases
			calculateFFTMagnitudes(
				buffer,
				sampleRate,
				FFT_ORDER,
				magnitudes,
				timeBinCenterSamples,
				binFrequencyResolution,
				numTimeBins,
				usePhaseExtraction ? &phases : nullptr,
				usePhaseExtraction,
				binsPerSecond);

			// Find maximum bin index from frequency
			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);

			// Extract dominant frequencies
			findDominantFrequenciesFromFFT(
				magnitudes,
				binFrequencyResolution,
				MIN_BIN,
				maxBin,
				dominantFrequencies,
				usePhaseExtraction ? &phases : nullptr,
				outPhaseTrajectory);
		}

		/// <summary>
		/// Calculates the number of time bins for FFT processing based on sample count and sample rate.
		/// Uses constant time resolution (binsPerSecond) to determine number of analysis frames.
		/// </summary>
		/// <param name="numSamples">Total number of audio samples</param>
		/// <param name="sampleRate">Sample rate in Hz</param>
		/// <param name="binsPerSecond">Desired time-frequency resolution in bins per second (default 512)</param>
		/// <returns>Number of time bins for FFT analysis (minimum 1)</returns>
		static int calculateNumTimeBins(int numSamples, int sampleRate, int binsPerSecond = BINS_PER_SECOND)
		{
			return std::max(1, binsPerSecond * (numSamples / sampleRate));
		}

		/// <summary>
		/// Applies median filter to smooth dominant frequencies to avoid rapid jumps.
		/// Overload for frequency values (Hz).
		/// </summary>
		/// <param name="inDominantFrequencies">Input vector of dominant frequencies in Hz</param>
		/// <param name="outSmoothedFrequencies">Output vector of smoothed frequencies in Hz</param>
		/// <param name="smoothingWindow">Window size for median filter (default 7)</param>
		static void smoothDominantFrequencies(
			const std::vector<float>& inDominantFrequencies,
			std::vector<float>& outSmoothedFrequencies,
			int smoothingWindow = 7)
		{
			if (inDominantFrequencies.empty())
			{
				outSmoothedFrequencies.clear();
				return;
			}

			outSmoothedFrequencies.resize(inDominantFrequencies.size());

			for (int timeIdx = 0; timeIdx < (int)inDominantFrequencies.size(); ++timeIdx)
			{
				std::vector<float> window;

				// Gather values within the smoothing window
				for (int i = -smoothingWindow / 2; i <= smoothingWindow / 2; ++i)
				{
					int idx = timeIdx + i;
					if (idx >= 0 && idx < (int)inDominantFrequencies.size())
					{
						window.push_back(inDominantFrequencies[idx]);
					}
				}

				// Sort and take median
				std::sort(window.begin(), window.end());
				outSmoothedFrequencies[timeIdx] = window[window.size() / 2];
			}
		}
	};
}
