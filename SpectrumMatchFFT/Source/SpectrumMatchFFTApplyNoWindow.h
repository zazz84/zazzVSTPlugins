#pragma once

#include <vector>
#include <complex>
#include <cmath>
#include <cstring>
#include <algorithm>

#include <JuceHeader.h>

/**
 * Applies spectrum adjustment to audio data without windowing
 * Designed for processing looped regions with direct spectrum matching
 */
class SpectrumMatchFFTApplyNoWindow
{
public:
	/** fftSize must be a power of two */
	explicit SpectrumMatchFFTApplyNoWindow(int fftSizeInSamples)
		: fftSize(fftSizeInSamples),
		fftOrder(static_cast<int>(std::log2(fftSizeInSamples))),
		numBins(fftSizeInSamples / 2 + 1),
		fft(fftOrder)
	{
		jassert(juce::isPowerOfTwo(fftSizeInSamples));

		fftData.resize(fftSize * 2);
		m_bucketGain.resize(numBins, 1.0f);
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
	 * Set spectrum gain for all frequency bins
	 */
	void setSpectrum(const std::vector<float>& spectrum)
	{
		jassert(spectrum.size() == numBins);
		std::copy(spectrum.begin(), spectrum.end(), m_bucketGain.begin());
	}

	/**
	 * Set spectrum gain for all frequency bins
	 */
	void setSpectrum(const float* spectrum, int size)
	{
		jassert(size == numBins);
		std::copy(spectrum, spectrum + numBins, m_bucketGain.begin());
	}

	/**
	 * Process a single frame without windowing (assumes looped data)
	 * @param data Audio data (must be fftSize samples)
	 */
	void processFrame(float* data)
	{
		jassert(data != nullptr);

		// Copy to FFT buffer (no windowing)
		std::copy(data, data + fftSize, fftData.begin());

		// Forward FFT
		fft.performRealOnlyForwardTransform(fftData.data(), true);

		// Apply spectrum adjustment
		applySpectrumGain(fftData.data());

		// Inverse FFT
		fft.performRealOnlyInverseTransform(fftData.data());

		// Normalize and copy back
		float invFFTScale = 1.0f / fftSize;
		for (int i = 0; i < fftSize; ++i)
		{
			data[i] = fftData[i] * invFFTScale;
		}
	}

	/**
	 * Process multiple frames from a buffer
	 * Processes buffer in chunks of fftSize without overlap
	 * @param data Audio buffer
	 * @param numSamples Number of samples to process
	 */
	void processBuffer(float* data, int numSamples)
	{
		jassert(data != nullptr);
		jassert(numSamples % fftSize == 0); // Must be multiple of fftSize

		for (int i = 0; i < numSamples; i += fftSize)
		{
			processFrame(data + i);
		}
	}

	/**
	 * Process from input to output
	 * @param input Input buffer (must be at least numSamples)
	 * @param output Output buffer (will be resized to numSamples)
	 * @param numSamples Number of samples
	 */
	void process(const float* input, std::vector<float>& output, int numSamples)
	{
		jassert(input != nullptr);
		jassert(numSamples % fftSize == 0);

		output.resize(numSamples);

		for (int i = 0; i < numSamples; i += fftSize)
		{
			std::copy(input + i, input + i + fftSize, fftData.begin());
			
			fft.performRealOnlyForwardTransform(fftData.data(), true);
			applySpectrumGain(fftData.data());
			fft.performRealOnlyInverseTransform(fftData.data());

			float invFFTScale = 1.0f / fftSize;
			for (int j = 0; j < fftSize; ++j)
			{
				output[i + j] = fftData[j] * invFFTScale;
			}
		}
	}

private:
	void applySpectrumGain(float* fftData)
	{
		auto* cdata = reinterpret_cast<std::complex<float>*>(fftData);

		for (int i = 0; i < numBins; ++i)
		{
			float magnitude = std::abs(cdata[i]);
			float phase = std::arg(cdata[i]);

			cdata[i] = std::polar(magnitude * m_bucketGain[i], phase);
		}
	}

private:
	const int fftSize;
	const int fftOrder;
	const int numBins;

	juce::dsp::FFT fft;

	std::vector<float> fftData;
	std::vector<float> m_bucketGain;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumMatchFFTApplyNoWindow)
};
