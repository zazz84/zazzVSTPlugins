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
		// Mathematical constants
		static constexpr float PI = 3.14159265358979323846f;
		static constexpr float TWO_PI = 2.0f * PI;

		// Frequency analysis constants
		static constexpr int MIN_BIN = 1;
		static constexpr float MAX_FREQUENCY = 400.0f;
		/// <summary>
		/// Wraps phase to [-π, π] range.
		/// </summary>
		/// <param name="phase">Phase value in radians</param>
		/// <returns>Wrapped phase in range [-π, π]</returns>
		static float wrapPhase(float phase)
		{
			while (phase > PI)
				phase -= TWO_PI;
			while (phase < -PI)
				phase += TWO_PI;

			return phase;
		}

		/// <summary>
		/// Unwraps phase difference to shortest path around the unit circle.
		/// </summary>
		/// <param name="phase1">Starting phase (radians)</param>
		/// <param name="phase2">Ending phase (radians)</param>
		/// <returns>Unwrapped difference that represents shortest path</returns>
		static float unwrapPhaseDifference(float phase1, float phase2)
		{
			float diff = phase2 - phase1;

			if (diff > PI)
				diff -= TWO_PI;
			else if (diff < -PI)
				diff += TWO_PI;

			return diff;
		}

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
			float diff = unwrapPhaseDifference(phase1, phase2);
			float result = phase1 + t * diff;
			return wrapPhase(result);
		}

		/// <summary>
		/// Calculates weighted average of phase values from multiple bins.
		/// Properly handles phase wrapping by unwrapping relative to center phase.
		/// Uses magnitude-weighted averaging for better stability.
		/// </summary>
		/// <param name="centerPhase">Phase at the center (dominant) bin</param>
		/// <param name="phasesAroundCenter">Array of phases from bins around center [-2,-1,0,+1,+2]</param>
		/// <param name="magnitudesAroundCenter">Array of magnitudes for weighting (same length as phases)</param>
		/// <param name="numBinsAround">Number of bins on each side of center (e.g., 2 for 5-bin window)</param>
		/// <returns>Weighted average phase in range [-π, π]</returns>
		static float averagePhaseFromWindow(
			float centerPhase,
			const float* phasesAroundCenter,
			const float* magnitudesAroundCenter,
			int numBinsAround)
		{
			if (numBinsAround <= 0)
				return centerPhase;

			// Start with center phase
			float sumWeightedPhase = 0.0f;
			float sumWeights = 0.0f;

			int centerIdx = numBinsAround;
			int totalBins = 2 * numBinsAround + 1;

			// Process all bins including center
			for (int i = 0; i < totalBins; ++i)
			{
				float phase = phasesAroundCenter[i];
				float magnitude = magnitudesAroundCenter[i];

				// Unwrap this phase relative to center phase
				float phaseDiff = unwrapPhaseDifference(centerPhase, phase);
				float unwrappedPhase = centerPhase + phaseDiff;

				// Weight by magnitude (higher magnitude = more influence)
				float weight = magnitude * magnitude;  // Square for stronger weighting
				sumWeightedPhase += unwrappedPhase * weight;
				sumWeights += weight;
			}

			// Calculate weighted average
			float averagePhase = sumWeights > 1e-6f ? sumWeightedPhase / sumWeights : centerPhase;
			return wrapPhase(averagePhase);
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
		/// <param name="inRealData">Optional input real FFT components for I/Q interpolation [timeIdx][freqBin]</param>
		/// <param name="inImagData">Optional input imaginary FFT components for I/Q interpolation [timeIdx][freqBin]</param>
		/// <param name="phaseWindowSize">Window size for phase averaging (0=single bin, 2=±2 bins around dominant, etc.)</param>
		static void findDominantFrequenciesFromFFT(
			const std::vector<std::vector<float>>& magnitudes,
			float binFrequencyResolution,
			int minBin,
			int maxBin,
			std::vector<float>& outDominantFrequencies,
			const std::vector<std::vector<float>>* inPhaseData = nullptr,
			std::vector<float>* outPhaseAtDominant = nullptr,
			const std::vector<std::vector<float>>* inRealData = nullptr,
			const std::vector<std::vector<float>>* inImagData = nullptr,
			int phaseWindowSize = 0)
		{
			const int NUM_TIME_BINS = magnitudes.size();
			outDominantFrequencies.assign(NUM_TIME_BINS, 0.0f);
			if (outPhaseAtDominant != nullptr)
			{
				outPhaseAtDominant->assign(NUM_TIME_BINS, 0.0f);
			}

			// Determine if we're using I/Q interpolation
			bool useIQInterpolation = (inRealData != nullptr && inImagData != nullptr && outPhaseAtDominant != nullptr);

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
				if (outPhaseAtDominant != nullptr && timeIdx < NUM_TIME_BINS)
				{
					if (useIQInterpolation && timeIdx < (int)inRealData->size() && timeIdx < (int)inImagData->size())
					{
						// I/Q interpolation: interpolate real and imaginary components separately, then calculate phase
						const int realSize = (int)(*inRealData)[timeIdx].size();
						const int imagSize = (int)(*inImagData)[timeIdx].size();

						if (maxFftBin < realSize && maxFftBin < imagSize)
						{
							if (phaseWindowSize > 0 && maxFftBin >= phaseWindowSize && maxFftBin + phaseWindowSize < realSize && maxFftBin + phaseWindowSize < imagSize)
							{
								// Use window-based phase averaging
								int windowSize = 2 * phaseWindowSize + 1;
								std::vector<float> windowPhases(windowSize);
								std::vector<float> windowMagnitudes(windowSize);

								// Extract phases and magnitudes from window
								for (int w = -phaseWindowSize; w <= phaseWindowSize; ++w)
								{
									int binIdx = maxFftBin + w;
									float real = (*inRealData)[timeIdx][binIdx];
									float imag = (*inImagData)[timeIdx][binIdx];
									float magnitude = std::sqrt(real * real + imag * imag);

									windowPhases[w + phaseWindowSize] = std::atan2(imag, real);
									windowMagnitudes[w + phaseWindowSize] = magnitude;
								}

								// Calculate weighted average phase
								(*outPhaseAtDominant)[timeIdx] = averagePhaseFromWindow(
									windowPhases[phaseWindowSize],
									windowPhases.data(),
									windowMagnitudes.data(),
									phaseWindowSize);
							}
							else
							{
								// Fall back to single-bin or sub-bin interpolation
								float interpolatedReal = (*inRealData)[timeIdx][maxFftBin];
								float interpolatedImag = (*inImagData)[timeIdx][maxFftBin];

								// If we have a sub-bin offset and neighboring bins, interpolate the I/Q values
								if (std::abs(binOffset) > 1e-6f)
								{
									if (binOffset > 0.0f && maxFftBin + 1 < realSize && maxFftBin + 1 < imagSize)
									{
										// Interpolate toward right neighbor using linear interpolation
										float rightReal = (*inRealData)[timeIdx][maxFftBin + 1];
										float rightImag = (*inImagData)[timeIdx][maxFftBin + 1];

										// Linear interpolation: result = a(1-t) + b*t
										float t = binOffset;
										interpolatedReal = interpolatedReal * (1.0f - t) + rightReal * t;
										interpolatedImag = interpolatedImag * (1.0f - t) + rightImag * t;
									}
									else if (binOffset < 0.0f && maxFftBin > 0)
									{
										// Interpolate toward left neighbor using linear interpolation
										float leftReal = (*inRealData)[timeIdx][maxFftBin - 1];
										float leftImag = (*inImagData)[timeIdx][maxFftBin - 1];

										// Linear interpolation with negative offset: result = a*abs(t) + b(1-abs(t))
										float t = -binOffset;  // Make positive
										interpolatedReal = leftReal * t + interpolatedReal * (1.0f - t);
										interpolatedImag = leftImag * t + interpolatedImag * (1.0f - t);
									}
								}

								// Calculate phase from interpolated I/Q - this is the raw phase from the spectrum
								(*outPhaseAtDominant)[timeIdx] = std::atan2(interpolatedImag, interpolatedReal);
							}
						}
					}
					else if (inPhaseData != nullptr && timeIdx < (int)inPhaseData->size())
					{
						// Legacy phase interpolation
						const int phaseSize = (int)(*inPhaseData)[timeIdx].size();
						if (maxFftBin < phaseSize)
						{
							if (phaseWindowSize > 0 && maxFftBin >= phaseWindowSize && maxFftBin + phaseWindowSize < phaseSize)
							{
								// Use window-based phase averaging
								int windowSize = 2 * phaseWindowSize + 1;
								std::vector<float> windowPhases(windowSize);
								std::vector<float> windowMagnitudes(windowSize);

								// Extract phases from window and get magnitudes
								for (int w = -phaseWindowSize; w <= phaseWindowSize; ++w)
								{
									int binIdx = maxFftBin + w;
									windowPhases[w + phaseWindowSize] = (*inPhaseData)[timeIdx][binIdx];
									windowMagnitudes[w + phaseWindowSize] = magnitudes[timeIdx][binIdx];
								}

								// Calculate weighted average phase
								(*outPhaseAtDominant)[timeIdx] = averagePhaseFromWindow(
									windowPhases[phaseWindowSize],
									windowPhases.data(),
									windowMagnitudes.data(),
									phaseWindowSize);
							}
							else
							{
								// Fall back to single-bin interpolation
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
		/// <param name="fftOrder">FFT order (default 12)</param>
		/// <param name="phaseWindowSize">Window size for phase averaging (0=single bin, 2=±2 bins around dominant, etc.)</param>
		static void calculateDominantFrequencies(
			const juce::AudioBuffer<float>& buffer,
			int sampleRate,
			std::vector<float>& dominantFrequencies,
			std::vector<int>& timeBinCenterSamples,
			std::vector<float>* outPhaseTrajectory = nullptr,
			bool usePhaseExtraction = false,
			int binsPerSecond = BINS_PER_SECOND,
			int fftOrder = 12,
			int phaseWindowSize = 0)
		{
			std::vector<std::vector<float>> magnitudes;
			std::vector<std::vector<float>> phases;
			float binFrequencyResolution = 0.0f;
			int numTimeBins = 0;

			// Calculate FFT magnitudes and optionally phases
			calculateFFTMagnitudes(
				buffer,
				sampleRate,
				fftOrder,
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
				outPhaseTrajectory,
				nullptr,
				nullptr,
				phaseWindowSize);
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
		/// Enhanced version that calculates dominant frequencies with accurate I/Q-based phase interpolation.
		/// Performs FFT on centered windows, extracts real/imaginary components, and interpolates them
		/// for sub-bin accurate phase calculation at dominant frequencies.
		/// 
		/// This method provides superior phase accuracy compared to direct phase interpolation because:
		/// - Interpolates real and imaginary components separately (linear interpolation is well-defined)
		/// - Calculates phase from interpolated I/Q values using atan2(Q, R)
		/// - Avoids phase wrapping issues that occur when interpolating phase directly
		/// - Optionally averages phase from multiple bins around the dominant for noise reduction
		/// </summary>
		/// <param name="buffer">Input audio buffer (single channel)</param>
		/// <param name="sampleRate">Sample rate of the audio buffer in Hz</param>
		/// <param name="dominantFrequencies">Output vector of dominant frequencies in Hz</param>
		/// <param name="timeBinCenterSamples">Output vector of center sample indices for each time bin</param>
		/// <param name="outPhaseTrajectory">Output vector of phase values at dominant frequencies (optional, calculated from interpolated I/Q)</param>
		/// <param name="binsPerSecond">Time-frequency resolution in bins per second (default 512)</param>
		/// <param name="fftOrder">FFT order (FFT_SIZE = 2^fftOrder). Default is 12 (4096 samples)</param>
		/// <param name="phaseWindowSize">Window size for phase averaging around dominant bin (0=single bin, 2=±2 bins for 5-bin average, etc.)</param>
		static void calculateDominantFrequenciesWithIQInterpolation(
			const juce::AudioBuffer<float>& buffer,
			int sampleRate,
			std::vector<float>& dominantFrequencies,
			std::vector<int>& timeBinCenterSamples,
			std::vector<float>* outPhaseTrajectory = nullptr,
			int binsPerSecond = BINS_PER_SECOND,
			int fftOrder = 12,
			int phaseWindowSize = 3)
		{
			constexpr int MIN_BIN = 1;
			constexpr float MAX_FREQUENCY = 400.0f;

			const int FFT_SIZE = 1 << fftOrder;
			const auto samples = buffer.getNumSamples();
			auto* pBuffer = buffer.getReadPointer(0);

			// Input buffer is too small for FFT
			if (samples < FFT_SIZE)
			{
				return;
			}

			const int NUM_TIME_BINS = calculateNumTimeBins(samples, sampleRate, binsPerSecond);

			juce::dsp::FFT forwardFFT(fftOrder);
			juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);

			const float binFrequencyResolution = (float)sampleRate / FFT_SIZE;
			const float blockSize = (float)samples / NUM_TIME_BINS;

			// Prepare output containers
			std::vector<std::vector<float>> magnitudes(NUM_TIME_BINS, std::vector<float>(FFT_SIZE / 2 + 1, 0.0f));
			std::vector<std::vector<float>> realData(NUM_TIME_BINS, std::vector<float>(FFT_SIZE / 2 + 1, 0.0f));
			std::vector<std::vector<float>> imagData(NUM_TIME_BINS, std::vector<float>(FFT_SIZE / 2 + 1, 0.0f));

			timeBinCenterSamples.assign(NUM_TIME_BINS, 0);

			std::vector<float> fftData(2 * FFT_SIZE);

			// Step 1: Perform FFT and extract magnitudes + real/imaginary components
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

				timeBinCenterSamples[timeIdx] = centerSample;

				// Clear FFT buffer
				std::fill(fftData.begin(), fftData.end(), 0.0f);

				// Copy audio data
				std::copy(pBuffer + startSample, pBuffer + startSample + FFT_SIZE, fftData.begin());

				// Apply windowing
				window.multiplyWithWindowingTable(fftData.data(), FFT_SIZE);

				// Perform FFT
				forwardFFT.performRealOnlyForwardTransform(fftData.data());

				// Extract magnitudes and real/imaginary components
				const int numBins = FFT_SIZE / 2 + 1;
				for (int bin = 0; bin < numBins; ++bin)
				{
					const float real = fftData[2 * bin];
					const float imag = fftData[2 * bin + 1];
					const float magnitude = std::sqrt(real * real + imag * imag);

					magnitudes[timeIdx][bin] = magnitude;
					realData[timeIdx][bin] = real;
					imagData[timeIdx][bin] = imag;
				}
			}

			// Step 2: Find dominant frequencies using I/Q interpolation for phase
			dominantFrequencies.assign(NUM_TIME_BINS, 0.0f);
			if (outPhaseTrajectory != nullptr)
			{
				outPhaseTrajectory->assign(NUM_TIME_BINS, 0.0f);
			}

			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);

			// Call findDominantFrequenciesFromFFT with I/Q data for superior phase accuracy
			findDominantFrequenciesFromFFT(
				magnitudes,
				binFrequencyResolution,
				MIN_BIN,
				maxBin,
				dominantFrequencies,
				nullptr,  // Don't use legacy phase interpolation
				outPhaseTrajectory,
				&realData,  // Pass real FFT components
				&imagData,  // Pass imaginary FFT components
				phaseWindowSize);  // Use window-based phase averaging
		}
	};
}