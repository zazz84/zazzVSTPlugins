#pragma once

#include <vector>
#include <JuceHeader.h>

namespace zazzGUI
{
	class WaveformDisplay: public juce::Component
	{
	public:
		WaveformDisplay() = default;
		~WaveformDisplay() = default;

		void setAudioBuffer(const juce::AudioBuffer<float> buffer)
		{
			m_audioBuffer = buffer;
			const int size = buffer.getNumSamples();

			m_leftSampleIndex = 0;
			m_rightSampleIndex = size;

			setVerticalNormalization();
			repaint();
		}

		void setVerticalZoom(const float verticalZoom)
		{
			m_verticalZoom = verticalZoom;
			repaint();
		}

		void setVerticalNormalization()
		{
			if (m_rightSampleIndex != 0)
			{
				const float magnitude = m_audioBuffer.getMagnitude(0, m_audioBuffer.getNumSamples());
				const float verticalZoom = (magnitude > 0.0f) ? 1.0f / magnitude : 1.0f;
				m_verticalZoom = verticalZoom;
			}
			else
			{
				m_verticalZoom = 1.0f;
			}
		}

		void setHorizontalZoom(const int leftSampleIndex, const int rightSampleIndex)
		{
			if (leftSampleIndex >= rightSampleIndex) return;  // Invalid range
			if (rightSampleIndex > m_audioBuffer.getNumSamples()) return;  // Out of bounds
			
			m_leftSampleIndex = std::max(0, leftSampleIndex);
			m_rightSampleIndex = rightSampleIndex;
			repaint();
		}

		void setWaveformColour(const juce::Colour colour)
		{
			m_waveformColour = colour;
			repaint();
		}

		void setWaveformThickness(const float thickness)
		{
			m_waveformThickness = thickness;
			repaint();
		}

		const juce::AudioBuffer<float>& getAudioBuffer() const
		{
			return m_audioBuffer;
		}

		void paint(juce::Graphics& g) override
		{
			const auto width = getWidth();
			const auto height = getHeight();

			g.setColour(juce::Colours::whitesmoke);
			g.drawLine(0.0f, (float)(height / 2), (float)width, (float)(height / 2), 1.0f);

			// Draw waveform
			if (m_audioBuffer.getNumSamples() != 0)
			{
				if (m_rightSampleIndex <= m_leftSampleIndex || m_rightSampleIndex > m_audioBuffer.getNumSamples())
					return;  // Skip rendering invalid state

				auto* channelData = m_audioBuffer.getReadPointer(0);

				juce::Path path;
				path.preallocateSpace(3 * (int)width);
				path.startNewSubPath(0.0f, (float)(height / 2));

				for (int x = 0; x < (int)width; ++x)
				{
					const auto sampleIndex = juce::jmap<int>(x, 0, (int)width, m_leftSampleIndex, m_rightSampleIndex - 1);
					const float level = m_verticalZoom * channelData[sampleIndex];
					float y = juce::jmap(level, -1.0f, 1.0f, (float)height, 0.0f);
					path.lineTo((float)x, y);
				}

				g.setColour(m_waveformColour);
				g.strokePath(path, juce::PathStrokeType(m_waveformThickness));
			}
		}

	private:
		juce::AudioBuffer<float> m_audioBuffer;

		float m_verticalZoom = 1.0f;
		float m_waveformThickness = 1.0f;
		juce::Colour m_waveformColour = juce::Colours::white;
		int m_leftSampleIndex = 0;
		int m_rightSampleIndex = 0;
	};
}