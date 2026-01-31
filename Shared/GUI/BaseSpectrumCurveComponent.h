#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

/**
	SpectrumCurveComponent
	Draws a logarithmic-frequency, dB-scaled spectrum.
	Input: linear FFT gains from SpectrumMatchFFTDetect
*/
class BaseSpectrumCurveComponent : public juce::Component
{
public:
	BaseSpectrumCurveComponent() = default;
	~BaseSpectrumCurveComponent() = default;

	void setSampleRate(int sampleRate)
	{
		m_sampleRate = sampleRate;

		updateFirstAndLastBin();
	}
	void setFFTSize(const int size)
	{
		m_fftSize = size;

		updateFirstAndLastBin();
	}
	void setMagnitudeRangeDb(float minDb, float maxDb)
	{
		m_minMagnitudeDb = minDb;
		m_maxMagnitudeDb = maxDb;
	}
	void setFrequencyRange(float minF, float maxF)
	{
		m_minFrequency = minF;
		m_maxFrequency = maxF;

		m_logMin = std::log10(minF);
		m_logMax = std::log10(maxF);

		updateFirstAndLastBin();
	}
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		constexpr int leftMargin = 20;
		constexpr int bottomMargin = 10;
		constexpr int topMargin = 20;
		constexpr int rightMargin = 30;

		auto plotArea = getLocalBounds()
			.reduced(rightMargin, topMargin)
			.withTrimmedLeft(leftMargin)
			.withTrimmedBottom(bottomMargin)
			.toFloat();

		drawGrid(g, plotArea);
		drawAxes(g, plotArea);
		drawSpectrum(g, plotArea);
	}

protected:
	float mapX(int bin, const juce::Rectangle<float>& area) const
	{
		const float freq =
			static_cast<float>(bin) * static_cast<float>(m_sampleRate)
			/ static_cast<float>(m_fftSize);

		const float normX =
			(std::log10(freq) - m_logMin) / (m_logMax - m_logMin);

		return area.getX() + normX * area.getWidth();
	}
	float mapY(float gain, const juce::Rectangle<float>& area) const
	{
		float db = juce::Decibels::gainToDecibels(gain);
		db = juce::jlimit(m_minMagnitudeDb, m_maxMagnitudeDb, db);

		const float norm =
			juce::jmap(db,
				m_minMagnitudeDb, m_maxMagnitudeDb,
				0.0f, 1.0f);

		return area.getBottom() - norm * area.getHeight();
	}
	void updateFirstAndLastBin()
	{
		const float temp = (float)m_fftSize / (float)m_sampleRate;
		m_firstBin = (int)(m_minFrequency * temp) + 1;
		m_lastBin = (int)(m_maxFrequency * temp);
	}
	void drawGrid(juce::Graphics& g, juce::Rectangle<float> area)
	{
		g.setColour(juce::Colours::darkgrey);

		// Horizontal lines (dB)
		constexpr int numYTicks = 5;
		for (int i = 1; i < numYTicks; ++i)
		{
			float y = area.getBottom() - i * area.getHeight() / numYTicks;
			g.drawHorizontalLine((int)y, area.getX(), area.getRight());
		}

		// Vertical lines (log-freq)
		const std::vector<float> freqs{ 20,50,100,200,500,1000,2000,5000,10000,20000 };
		for (float freq : freqs)
		{
			if (freq < m_minFrequency || freq > m_maxFrequency)
				continue;

			float normX = (std::log10(freq) - m_logMin) / (m_logMax - m_logMin);
			float x = area.getX() + normX * area.getWidth();
			g.drawVerticalLine((int)x, area.getY(), area.getBottom());
		}
	}
	void drawAxes(juce::Graphics& g, juce::Rectangle<float> area)
	{
		g.setColour(juce::Colours::white);
		g.setFont(10.0f);

		// ==== Y AXIS (dB) ====
		constexpr int numYTicks = 5;
		for (int i = 0; i <= numYTicks; ++i)
		{
			float norm = (float)i / numYTicks;
			float y = area.getBottom() - norm * area.getHeight();

			float dbValue = juce::jmap(norm, 0.0f, 1.0f,
				m_minMagnitudeDb, m_maxMagnitudeDb);

			g.drawText(juce::String((int)dbValue) + " dB",
				2, (int)y - 7, 45, 14,
				juce::Justification::centred);
		}

		// ==== X AXIS (log-frequency) ====
		const std::vector<float> freqs{ 20,50,100,200,500,1000,2000,5000,10000,20000 };
		for (float freq : freqs)
		{
			if (freq < m_minFrequency || freq > m_maxFrequency)
				continue;

			float normX = (std::log10(freq) - m_logMin) / (m_logMax - m_logMin);
			float x = area.getX() + normX * area.getWidth();

			juce::String label = freq < 1000
				? juce::String((int)freq) + " Hz"
				: juce::String(freq / 1000.0f, 0) + " kHz";

			g.drawText(label,
				(int)x - 30,
				area.getBottom() + 5,
				60, 20,
				juce::Justification::centred);
		}
	}
	virtual void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area)
	{
	}

	float m_minMagnitudeDb = -80.0f;
	float m_maxMagnitudeDb = 12.0f;

	float m_minFrequency = 20.0f;
	float m_maxFrequency = 20000.0f;
	float m_logMin = std::log10(20.0f);
	float m_logMax = std::log10(20000.0f);

	int m_sampleRate = 48000;
	int m_fftSize = 256;
	int m_firstBin = 1;
	int m_lastBin = 106;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseSpectrumCurveComponent)
};
