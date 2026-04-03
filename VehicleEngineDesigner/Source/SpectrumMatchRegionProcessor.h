#pragma once

#include <vector>
#include <complex>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <JuceHeader.h>

/**
 * Processes audio regions to calculate and apply average spectrum matching.
 * 
 * Features:
 * - Calculates average spectrum from valid regions
 * - Applies spectrum adjustment to regions without windowing (assumes looped regions)
 * - Resamples regions to/from FFT size for processing
 * - Supports region export and output buffer generation
 */
class SpectrumMatchRegionProcessor
{
public:
	/** fftSize must be a power of two */
	explicit SpectrumMatchRegionProcessor(int fftSizeInSamples)
		: fftSize(fftSizeInSamples),
		fftOrder(static_cast<int>(std::log2(fftSizeInSamples))),
		numBins(fftSizeInSamples / 2 + 1),
		fft(fftOrder)
	{
		jassert(juce::isPowerOfTwo(fftSizeInSamples));

		fftData.resize(fftSize * 2);
		m_averageSpectrum.resize(numBins, 1.0f);
		m_spectrumGain.resize(numBins, 1.0f);
		m_tempSpectrum.resize(numBins, 0.0f);
	}

	int getFFTSize() const
	{
		return fftSize;
	}

	int getNumBins() const
	{
		return numBins;
	}

	/**
	 * Calculate average spectrum from valid regions
	 * @param regions Vector of region sizes (in samples)
	 * @param audioData Pointer to the full audio buffer
	 * @param regionStartPositions Vector of start sample positions for each region
	 */
	void calculateAverageSpectrum(const std::vector<int>& regions, 
									const float* audioData,
									const std::vector<int>& regionStartPositions)
	{
		jassert(regions.size() == regionStartPositions.size());

		// Clear average spectrum accumulator
		std::fill(m_tempSpectrum.begin(), m_tempSpectrum.end(), 0.0f);
		int spectrumCount = 0;

		// Process each valid region
		for (size_t i = 0; i < regions.size(); ++i)
		{
			if (regions[i] > 0)
			{
				processRegionForSpectrum(audioData + regionStartPositions[i], 
										regions[i], 
										m_tempSpectrum);
				++spectrumCount;
			}
		}

		// Calculate normalized average spectrum
		if (spectrumCount > 0)
		{
			float normalization = 1.0f / spectrumCount;
			for (int i = 0; i < numBins; ++i)
			{
				m_averageSpectrum[i] = m_tempSpectrum[i] * normalization;
			}
		}
		else
		{
			std::fill(m_averageSpectrum.begin(), m_averageSpectrum.end(), 1.0f);
		}
	}

	/**
	 * Calculate median spectrum from valid regions
	 * @param regions Vector of region sizes (in samples)
	 * @param audioData Pointer to the full audio buffer
	 * @param regionStartPositions Vector of start sample positions for each region
	 */
	void calculateMedianSpectrum(const std::vector<int>& regions, 
								 const float* audioData,
								 const std::vector<int>& regionStartPositions)
	{
		jassert(regions.size() == regionStartPositions.size());

		// Collect spectra from all valid regions
		std::vector<std::vector<float>> spectra;

		for (size_t i = 0; i < regions.size(); ++i)
		{
			if (regions[i] > 0)
			{
				std::vector<float> spectrum(numBins, 0.0f);
				processRegionForSpectrum(audioData + regionStartPositions[i], 
										regions[i], 
										spectrum);
				spectra.push_back(spectrum);
			}
		}

		// Calculate median spectrum
		if (spectra.size() > 0)
		{
			// For each frequency bin, calculate the median across all regions
			for (int binIdx = 0; binIdx < numBins; ++binIdx)
			{
				std::vector<float> binValues;
				for (const auto& spectrum : spectra)
				{
					binValues.push_back(spectrum[binIdx]);
				}

				// Sort to find median
				std::sort(binValues.begin(), binValues.end());

				if (binValues.size() % 2 == 1)
				{
					// Odd number of values - take middle value
					m_averageSpectrum[binIdx] = binValues[binValues.size() / 2];
				}
				else
				{
					// Even number of values - take average of two middle values
					int mid = binValues.size() / 2;
					m_averageSpectrum[binIdx] = (binValues[mid - 1] + binValues[mid]) * 0.5f;
				}
			}
		}
		else
		{
			std::fill(m_averageSpectrum.begin(), m_averageSpectrum.end(), 1.0f);
		}
	}

	/**
	 * Calculate spectrum from a specific region
	 * @param regionData Audio data for the region
	 * @param regionSize Size of the region
	 */
	void calculateRegionSpectrum(const float* regionData, int regionSize)
	{
		jassert(regionData != nullptr);
		jassert(regionSize > 0);

		// Clear and extract spectrum from single region
		std::fill(m_averageSpectrum.begin(), m_averageSpectrum.end(), 0.0f);
		std::fill(m_tempSpectrum.begin(), m_tempSpectrum.end(), 0.0f);

		processRegionForSpectrum(regionData, regionSize, m_tempSpectrum);

		// Use this region's spectrum as the target (no averaging)
		std::copy(m_tempSpectrum.begin(), m_tempSpectrum.end(), m_averageSpectrum.begin());
	}

	/**
	 * Calculate spectrum gain from detected spectrum to match average
	 * @param detectedSpectrum The detected spectrum from SpectrumMatchFFTDetect
	 */
	void calculateSpectrumGain(const float* detectedSpectrum)
	{
		jassert(detectedSpectrum != nullptr);

		for (int i = 0; i < numBins; ++i)
		{
			if (detectedSpectrum[i] > 1e-6f)
			{
				m_spectrumGain[i] = m_averageSpectrum[i] / detectedSpectrum[i];
			}
			else
			{
				m_spectrumGain[i] = 1.0f;
			}
		}
	}

	/**
	 * Apply spectrum adjustment to a region
	 * Resamples region to FFT size, applies spectrum without windowing, resamples back
	 * 
	 * @param inputRegion Input audio region
	 * @param inputSize Input region size
	 * @param outputRegion Output buffer (should be same size as input)
	 * @param intensity Spectrum matching intensity (0.0 = no adjustment, 1.0 = full ±24dB adjustment)
	 */
	void applySpectrumAdjustment(const float* inputRegion, int inputSize, float* outputRegion, float intensity = 1.0f)
	{
		jassert(inputRegion != nullptr);
		jassert(outputRegion != nullptr);
		jassert(inputSize > 0);

		// Resample input to FFT size
		std::vector<float> resampledInput(fftSize);
		resampleToFFTSize(inputRegion, inputSize, resampledInput.data());

		// Copy to FFT buffer (no windowing - assume looped region)
		std::copy(resampledInput.begin(), resampledInput.end(), fftData.begin());

		// Perform FFT
		fft.performRealOnlyForwardTransform(fftData.data(), true);

		// Apply spectrum adjustment with intensity scaling
		applySpectrumGain(fftData.data(), intensity);

		// Perform inverse FFT
		fft.performRealOnlyInverseTransform(fftData.data());

		// Resample back to original size
		resampleFromFFTSize(fftData.data(), outputRegion, inputSize);
	}

	/**
	 * Export a spectrum-adjusted region
	 * @param inputRegion Input audio region
	 * @param inputSize Input region size
	 * @param outputBuffer Output buffer for the adjusted region
	 */
	void exportAdjustedRegion(const float* inputRegion, int inputSize, std::vector<float>& outputBuffer)
	{
		outputBuffer.resize(inputSize);
		applySpectrumAdjustment(inputRegion, inputSize, outputBuffer.data());
	}

	/**
	 * Add spectrum-adjusted region to output buffer at specified position
	 * @param inputRegion Input audio region
	 * @param inputSize Input region size
	 * @param outputBuffer Output buffer to add to
	 * @param outputPosition Position in output buffer to write to
	 * @param outputBufferSize Total size of output buffer
	 */
	void addAdjustedRegionToOutput(const float* inputRegion, int inputSize, 
								   float* outputBuffer, int outputPosition, int outputBufferSize)
	{
		jassert(outputPosition + inputSize <= outputBufferSize);

		std::vector<float> adjustedRegion(inputSize);
		applySpectrumAdjustment(inputRegion, inputSize, adjustedRegion.data());

		// Add to output buffer
		for (int i = 0; i < inputSize; ++i)
		{
			outputBuffer[outputPosition + i] += adjustedRegion[i];
		}
	}

	/**
	 * Get the current average spectrum
	 */
	const std::vector<float>& getAverageSpectrum() const
	{
		return m_averageSpectrum;
	}

	/**
	 * Get the current spectrum gain
	 */
	const std::vector<float>& getSpectrumGain() const
	{
		return m_spectrumGain;
	}

private:
	/**
	 * Process a single region to extract its spectrum magnitude
	 */
	void processRegionForSpectrum(const float* regionData, int regionSize, std::vector<float>& spectrumAccumulator)
	{
		std::vector<float> resampledRegion(fftSize);
		resampleToFFTSize(regionData, regionSize, resampledRegion.data());

		// Copy to FFT buffer
		std::copy(resampledRegion.begin(), resampledRegion.end(), fftData.begin());

		// Perform FFT
		fft.performRealOnlyForwardTransform(fftData.data(), true);

		// Extract magnitudes and accumulate
		auto* cdata = reinterpret_cast<std::complex<float>*>(fftData.data());
		for (int i = 0; i < numBins; ++i)
		{
			spectrumAccumulator[i] += std::abs(cdata[i]);
		}
	}

	/**
	 * Apply spectrum gain to FFT data in-place with intensity scaling
	 * Scales each frequency bin by the ratio of average spectrum to current spectrum,
	 * with the adjustment amount controlled by intensity (0.0 = no adjustment, 1.0 = full adjustment)
	 */
	void applySpectrumGain(float* fftData, float intensity = 1.0f)
	{
		auto* cdata = reinterpret_cast<std::complex<float>*>(fftData);

		// Apply gain to each frequency bin
		for (int i = 0; i < numBins; ++i)
		{
			float magnitude = std::abs(cdata[i]);
			float phase = std::arg(cdata[i]);

			// Calculate the gain for this bin: ratio of average spectrum to current magnitude
			// This adjusts the spectrum to match the average while preserving relative amplitudes
			float gain = 1.0f;
			if (magnitude > 1e-6f)
			{
				// Scale by the ratio: average_spectrum / current_magnitude
				// This makes quiet bins louder and loud bins quieter to match the average profile
				float targetGain = m_averageSpectrum[i] / magnitude;

				// Limit gain to prevent extreme amplification (±24 dB range)
				const float maxGain = 15.85f;  // +24 dB
				const float minGain = 0.0631f; // -24 dB
				targetGain = juce::jlimit(minGain, maxGain, targetGain);

				// Interpolate between unity gain (no adjustment) and target gain based on intensity
				// gain = 1.0 + (targetGain - 1.0) * intensity
				gain = 1.0f + (targetGain - 1.0f) * intensity;
			}

			// Apply gain while preserving phase
			cdata[i] = std::polar(magnitude * gain, phase);
		}
	}

	/**
	 * Linear interpolation-based resampling to FFT size
	 */
	void resampleToFFTSize(const float* input, int inputSize, float* output)
	{
		if (inputSize == fftSize)
		{
			std::copy(input, input + fftSize, output);
			return;
		}

		float ratio = static_cast<float>(inputSize - 1) / (fftSize - 1);

		for (int i = 0; i < fftSize; ++i)
		{
			float srcPos = i * ratio;
			int srcIdx = static_cast<int>(srcPos);
			float frac = srcPos - srcIdx;

			// Linear interpolation with wrapping (looped region)
			int nextIdx = (srcIdx + 1) % inputSize;
			float sample0 = input[srcIdx];
			float sample1 = input[nextIdx];

			output[i] = sample0 + frac * (sample1 - sample0);
		}
	}

	/**
	 * Linear interpolation-based resampling from FFT size back to original size
	 */
	void resampleFromFFTSize(const float* input, float* output, int outputSize)
	{
		if (outputSize == fftSize)
		{
			std::copy(input, input + fftSize, output);
			return;
		}

		float ratio = static_cast<float>(fftSize - 1) / (outputSize - 1);

		for (int i = 0; i < outputSize; ++i)
		{
			float srcPos = i * ratio;
			int srcIdx = static_cast<int>(srcPos);
			float frac = srcPos - srcIdx;

			// Linear interpolation (input is already sized to fftSize, no wrapping needed)
			int nextIdx = std::min(srcIdx + 1, fftSize - 1);
			float sample0 = input[srcIdx];
			float sample1 = input[nextIdx];

			output[i] = sample0 + frac * (sample1 - sample0);
		}
	}

private:
	const int fftSize;
	const int fftOrder;
	const int numBins;

	juce::dsp::FFT fft;

	std::vector<float> fftData;
	std::vector<float> m_averageSpectrum;
	std::vector<float> m_spectrumGain;
	std::vector<float> m_tempSpectrum;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumMatchRegionProcessor)
};
