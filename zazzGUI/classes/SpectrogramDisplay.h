#pragma once

#include <vector>
#include <JuceHeader.h>

namespace zazzGUI
{
	class SpectrogramDisplay : public juce::Component, public juce::TooltipClient
	{
	public:
		SpectrogramDisplay(juce::String name) : m_nameGroupComponent(name)
		{
			addAndMakeVisible(m_nameGroupComponent);
		}

		~SpectrogramDisplay() = default;

		juce::String getTooltip() override
		{
			return "";  // Tooltip is drawn in paint() instead of using the tooltip system
		}

		void setAudioBuffer(const juce::AudioBuffer<float> buffer)
		{
			m_audioBuffer = buffer;
			m_leftSampleIndex = 0;
			m_rightSampleIndex = buffer.getNumSamples();
			
			computeSpectrogram();
			updateSpectrogramImage();
		}

		void setSampleRate(const int sampleRate)
		{
			if (sampleRate > 0)
			{
				m_sampleRate = sampleRate;
			}
		}

		void setHorizontalZoom(const int leftSampleIndex, const int rightSampleIndex)
		{
			if (leftSampleIndex >= rightSampleIndex) return;  // Invalid range
			if (rightSampleIndex > m_audioBuffer.getNumSamples()) return;  // Out of bounds

			m_leftSampleIndex = std::max(0, leftSampleIndex);
			m_rightSampleIndex = rightSampleIndex;

			// Recompute spectrogram for the zoomed region if it's large enough
			const int zoomSamples = m_rightSampleIndex - m_leftSampleIndex;
			if (zoomSamples >= FFT_SIZE)
			{
				// Zoom region is large enough, recompute spectrogram for better resolution
				computeSpectrogramForZoomedRegion();
			}
			// else: use the pre-computed full buffer spectrogram, just display a zoomed portion

			updateSpectrogramImage();
		}

		void computeSpectrogramForZoomedRegion()
		{
			m_spectrogram.validTimeSteps = 0;
			m_spectrogram.maxMagnitude = 0.0f;
			m_spectrogram.frameCenterSamples.clear();

			const int zoomSamples = m_rightSampleIndex - m_leftSampleIndex;
			if (zoomSamples <= 0 || m_audioBuffer.getNumSamples() == 0)
			{
				return;
			}

			// Create a temporary buffer containing only the zoomed region
			juce::AudioBuffer<float> zoomedBuffer(m_audioBuffer.getNumChannels(), zoomSamples);
			for (int channel = 0; channel < m_audioBuffer.getNumChannels(); ++channel)
			{
				const auto* sourceData = m_audioBuffer.getReadPointer(channel, m_leftSampleIndex);
				auto* destData = zoomedBuffer.getWritePointer(channel);
				std::copy(sourceData, sourceData + zoomSamples, destData);
			}

			// Calculate FFT magnitudes using centered windows on the zoomed buffer
			std::vector<std::vector<float>> magnitudes;
			std::vector<int> frameCenterSamples;
			float binFrequencyResolution = 0.0f;
			int numTimeBins = 0;

			zazzDSP::Spectrum::calculateFFTMagnitudes(
				zoomedBuffer,
				m_sampleRate,
				FFT_ORDER,
				magnitudes,
				frameCenterSamples,
				binFrequencyResolution,
				numTimeBins);

			if (numTimeBins == 0 || magnitudes.empty())
			{
				return;
			}

			// Store frame center samples, converting from zoomed buffer coordinates to original buffer coordinates
			m_spectrogram.frameCenterSamples.resize(frameCenterSamples.size());
			for (size_t i = 0; i < frameCenterSamples.size(); ++i)
			{
				m_spectrogram.frameCenterSamples[i] = frameCenterSamples[i] + m_leftSampleIndex;
			}

			// Remap FFT bins to display frequency range and normalize
			remapFFTBinsToDisplay(magnitudes, binFrequencyResolution, numTimeBins);
		}

		int getLeftSampleIndex() const
		{
			return m_leftSampleIndex;
		}

		int getRightSampleIndex() const
		{
			return m_rightSampleIndex;
		}

	private:
		static constexpr int FFT_ORDER = 14;
		static constexpr int FFT_SIZE = 1 << FFT_ORDER;
		static constexpr float MIN_FREQUENCY = 10.0f;
		static constexpr int NUM_FREQUENCY_BINS = 256;
		static constexpr float MAX_FREQUENCY = 400.0f;
		static constexpr int NUM_TIME_BINS = 2048;

		struct SpectrogramData
		{
			std::vector<std::vector<float>> data;
			std::vector<int> frameCenterSamples;  // Maps time bin index to sample position
			int validTimeSteps = 0;
			float maxMagnitude = 0.0f;

			SpectrogramData() : data(NUM_TIME_BINS, std::vector<float>(NUM_FREQUENCY_BINS, 0.0f)) {}
		};

		void computeSpectrogram()
		{
			m_spectrogram.validTimeSteps = 0;
			m_spectrogram.maxMagnitude = 0.0f;
			m_spectrogram.frameCenterSamples.clear();

			if (m_audioBuffer.getNumSamples() == 0)
			{
				return;
			}

			// Calculate FFT magnitudes using centered windows
			std::vector<std::vector<float>> magnitudes;
			std::vector<int> frameCenterSamples;
			float binFrequencyResolution = 0.0f;
			int numTimeBins = 0;

			zazzDSP::Spectrum::calculateFFTMagnitudes(
				m_audioBuffer,
				m_sampleRate,
				FFT_ORDER,
				magnitudes,
				frameCenterSamples,
				binFrequencyResolution,
				numTimeBins);

			if (numTimeBins == 0 || magnitudes.empty())
			{
				return;
			}

			// Store frame center samples for later zoom calculations
			m_spectrogram.frameCenterSamples = frameCenterSamples;

			// Remap FFT bins to display frequency range and normalize
			remapFFTBinsToDisplay(magnitudes, binFrequencyResolution, numTimeBins);
		}

		void remapFFTBinsToDisplay(const std::vector<std::vector<float>>& magnitudes, float binFrequencyResolution, int numTimeBins)
		{
			// Map FFT bins to display frequency range
			const int minBin = (int)(MIN_FREQUENCY / binFrequencyResolution);
			const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);
			const int freqBinRange = maxBin - minBin;

			float maxMagnitudeLocal = 0.0f;

			// First pass: map to display range and find maximum magnitude
			for (int timeIdx = 0; timeIdx < numTimeBins; ++timeIdx)
			{
				for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
				{
					const int fftBin = minBin + (freqIdx * freqBinRange) / NUM_FREQUENCY_BINS;

					if (fftBin >= 0 && fftBin < (int)magnitudes[timeIdx].size())
					{
						const float magnitude = magnitudes[timeIdx][fftBin];
						m_spectrogram.data[timeIdx][freqIdx] = magnitude;
						maxMagnitudeLocal = std::max(maxMagnitudeLocal, magnitude);
					}
				}

				m_spectrogram.validTimeSteps++;
			}

			// Second pass: normalize all values based on maximum magnitude
			m_spectrogram.maxMagnitude = maxMagnitudeLocal;

			if (maxMagnitudeLocal > 0.0f)
			{
				for (int timeIdx = 0; timeIdx < m_spectrogram.validTimeSteps; ++timeIdx)
				{
					for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
					{
						m_spectrogram.data[timeIdx][freqIdx] /= maxMagnitudeLocal;
						m_spectrogram.data[timeIdx][freqIdx] = juce::jlimit(0.0f, 1.0f, m_spectrogram.data[timeIdx][freqIdx]);
					}
				}
			}
		}

		
		void updateSpectrogramImage()
		{
			const int width = getWidth();
			const int height = getHeight();

			if (width <= 0 || height <= 0)
			{
				return;
			}

			// Create image to cache the spectrogram
			m_spectrogramImage = juce::Image(juce::Image::RGB, width, height, true);
			juce::Graphics g(m_spectrogramImage);

			// Draw spectrogram
			if (m_spectrogram.validTimeSteps > 0 && !m_spectrogram.frameCenterSamples.empty())
			{
				// Find the time bin range that corresponds to the zoom sample range
				// Use lower_bound and upper_bound for robust searching
				int startTimeBin = 0;
				int endTimeBin = m_spectrogram.validTimeSteps - 1;

				// Find first frame center that is >= m_leftSampleIndex
				auto it = std::lower_bound(m_spectrogram.frameCenterSamples.begin(), m_spectrogram.frameCenterSamples.end(), m_leftSampleIndex);
				if (it != m_spectrogram.frameCenterSamples.end())
				{
					startTimeBin = std::distance(m_spectrogram.frameCenterSamples.begin(), it);
				}
				else
				{
					// If no frame >= m_leftSampleIndex, use the last frame
					startTimeBin = m_spectrogram.frameCenterSamples.size() - 1;
				}

				// Find last frame center that is <= m_rightSampleIndex
				auto it_upper = std::upper_bound(m_spectrogram.frameCenterSamples.begin(), m_spectrogram.frameCenterSamples.end(), m_rightSampleIndex);
				if (it_upper != m_spectrogram.frameCenterSamples.begin())
				{
					endTimeBin = std::distance(m_spectrogram.frameCenterSamples.begin(), std::prev(it_upper));
				}
				else
				{
					// If no frame <= m_rightSampleIndex, use the first frame
					endTimeBin = 0;
				}

				// Ensure valid range
				startTimeBin = std::max(0, std::min(startTimeBin, (int)m_spectrogram.frameCenterSamples.size() - 1));
				endTimeBin = std::max(0, std::min(endTimeBin, (int)m_spectrogram.frameCenterSamples.size() - 1));

				const int numDisplayBins = endTimeBin - startTimeBin + 1;
				if (numDisplayBins <= 0) return;

				const float pixelWidth = (float)width / (float)numDisplayBins;
				const float pixelHeight = (float)height / (float)NUM_FREQUENCY_BINS;

				for (int displayIdx = 0; displayIdx < numDisplayBins; ++displayIdx)
				{
					const int timeIdx = startTimeBin + displayIdx;
					if (timeIdx < 0 || timeIdx >= m_spectrogram.validTimeSteps) continue;

					for (int freqIdx = 0; freqIdx < NUM_FREQUENCY_BINS; ++freqIdx)
					{
						const float value = m_spectrogram.data[timeIdx][NUM_FREQUENCY_BINS - 1 - freqIdx];
						const juce::Colour colour = getColourForValue(value);

						g.setColour(colour);
						const float x = (float)displayIdx * pixelWidth;
						const float y = (float)freqIdx * pixelHeight;
						g.fillRect(x, y, pixelWidth, pixelHeight);
					}
				}
			}
		}

		void paint(juce::Graphics& g) override
		{
			// Draw cached spectrogram image (only for Spectrogram and DominantFrequency modes)
			if (!m_spectrogramImage.isNull())
			{
				g.setOpacity(1.0f);
				g.drawImageAt(m_spectrogramImage, 0, 0);
			}

			// Draw tooltip in the bottom right corner with frequency and samples on separate lines
			if (m_tooltipFrequency >= 0.0f)
			{
				const int samplesPerCycle = (int)(m_sampleRate / m_tooltipFrequency);

				g.setColour(juce::Colours::white);
				g.setFont(12.0f);

				const int tooltipX = getWidth() - 100;
				const int tooltipY = getHeight() - 35;
				const int tooltipWidth = 95;
				const int lineHeight = 15;

				// Draw frequency on first line
				juce::String frequencyLine = juce::String(m_tooltipFrequency, 1) + " Hz";
				g.drawText(frequencyLine, tooltipX, tooltipY, tooltipWidth, lineHeight, juce::Justification::right);

				// Draw samples on second line
				juce::String samplesLine = juce::String(samplesPerCycle) + " samples";
				g.drawText(samplesLine, tooltipX, tooltipY + lineHeight, tooltipWidth, lineHeight, juce::Justification::right);
			}
		}

		void resized() override
		{
			const auto width = getWidth();
			const auto height = getHeight();
			const auto pixelSize = height / 11;

			// Set size
			m_nameGroupComponent.setSize(width, pixelSize);

			// Set position
			const auto column1 = 0;
			const auto row1 = 0;

			m_nameGroupComponent.setTopLeftPosition(column1, row1);

			// Regenerate cached spectrogram image when component is resized
			updateSpectrogramImage();
		}

		void mouseMove(const juce::MouseEvent& event) override
		{
			// Calculate spectrogram display area bounds
			const int pixelSize = getHeight() / 11;
			const int spectrogramX = 0;
			const int spectrogramY = pixelSize;
			const int spectrogramWidth = getWidth();
			const int spectrogramHeight = getHeight() - pixelSize;

			// Check if mouse is over the spectrogram display area
			if (event.x >= spectrogramX && event.x < spectrogramX + spectrogramWidth &&
				event.y >= spectrogramY && event.y < spectrogramY + spectrogramHeight)
			{
				// Calculate which frequency bin this pixel corresponds to
				const float relativY = (float)(event.y - spectrogramY);
				const int freqIdx = (int)((relativY / (float)spectrogramHeight) * NUM_FREQUENCY_BINS);
				const int clampedFreqIdx = juce::jlimit(0, NUM_FREQUENCY_BINS - 1, freqIdx);

				// Convert pixel index back to FFT bin using the same logic as computeSpectrogram
				const float binFrequencyResolution = (float)m_sampleRate / FFT_SIZE;
				const int minBin = (int)(MIN_FREQUENCY / binFrequencyResolution);
				const int maxBin = (int)(MAX_FREQUENCY / binFrequencyResolution);
				const int freqBinRange = maxBin - minBin;

				// Map from pixel index to FFT bin (accounting for display inversion)
				const int displayFreqIdx = NUM_FREQUENCY_BINS - 1 - clampedFreqIdx;
				const int fftBin = minBin + (displayFreqIdx * freqBinRange) / NUM_FREQUENCY_BINS;

				// Convert FFT bin back to frequency
				m_tooltipFrequency = fftBin * binFrequencyResolution;
			}
			else
			{
				m_tooltipFrequency = -1.0f;  // Invalid frequency to indicate no tooltip
			}

			repaint();  // Only repaint the tooltip overlay, spectrogram is cached
		}

		juce::Colour getColourForValue(float value)
		{
			// Gradient from dark navy blue (value=0) to yellow/orange (value=1)
			// Clamp value to [0, 1] range
			const float clampedValue = juce::jlimit(0.0f, 1.0f, value);

			// Navy blue: H=0.65 (blue), S=0.9, V=0.6
			// Yellow/Orange: H=0.1, S=0.95, V=1.0
			const float hueStart = 0.65f;    // Navy blue
			const float hueEnd = 0.1f;       // Yellow/Orange
			const float satStart = 0.9f;
			const float satEnd = 0.95f;
			const float brightStart = 0.6f;
			const float brightEnd = 1.0f;

			// Interpolate HSV values
			const float hue = hueStart + (hueEnd - hueStart) * clampedValue;
			const float saturation = satStart + (satEnd - satStart) * clampedValue;
			const float brightness = brightStart + (brightEnd - brightStart) * clampedValue;

			return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
		}

		juce::AudioBuffer<float> m_audioBuffer;
		SpectrogramData m_spectrogram;
		zazzGUI::GroupLabel m_nameGroupComponent;
		juce::Image m_spectrogramImage;  // Cached spectrogram rendering
		float m_tooltipFrequency = -1.0f;  // Current frequency at mouse position (-1 = no tooltip)
		std::vector<int> m_dominantFrequencyBins;  // Smoothed dominant frequency bins for each time step
		int m_sampleRate = 48000;  // Audio sample rate in Hz (default 48kHz)
		int m_leftSampleIndex = 0;
		int m_rightSampleIndex = 0;

	};
}