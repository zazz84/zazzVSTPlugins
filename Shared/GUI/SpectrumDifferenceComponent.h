#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

/**
	SpectrumDifferenceComponent
	Draws a logarithmic-frequency, dB-scaled spectrum.
	Input: linear FFT gains from SpectrumMatchFFTDetect
*/
class SpectrumDifferenceComponent : public juce::Component
{
public:
	SpectrumDifferenceComponent() = default;

	//==============================================================================
	void setSpectrum(const std::vector<float>& spectrum)
	{
		m_spectrum.resize(spectrum.size());
		std::copy(spectrum.begin(), spectrum.end(), m_spectrum.begin());
		repaint();
	}

	void setSampleRate(double newSampleRate)
	{
		sampleRate = newSampleRate;
	}

	void setFFTSize(const int size)
	{
		fftSize = size;
	}

	void setMagnitudeRangeDb(float minDb, float maxDb)
	{
		minMagnitudeDb = minDb;
		maxMagnitudeDb = maxDb;
	}

	void setFrequencyRange(float minF, float maxF)
	{
		minFrequency = minF;
		maxFrequency = maxF;

		logMin = std::log10(minFrequency);
		logMax = std::log10(maxFrequency);
	}

	//==============================================================================
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		constexpr int leftMargin = 10;
		constexpr int bottomMargin = 10;
		constexpr int topMargin = 20;
		constexpr int rightMargin = 40;

		auto plotArea = getLocalBounds()
			.reduced(rightMargin, topMargin)
			.withTrimmedLeft(leftMargin)
			.withTrimmedBottom(bottomMargin)
			.toFloat();

		drawGrid(g, plotArea);
		drawAxes(g, plotArea);

		if (!m_spectrum.empty())
		{
			drawSpectrum(g, plotArea);;
		}
	}

private:
	//==============================================================================
	void drawGrid(juce::Graphics& g, juce::Rectangle<float> area)
	{
		g.setColour(juce::Colours::darkgrey);

		// Horizontal lines (dB)
		constexpr int numYTicks = 6;
		for (int i = 1; i < numYTicks; ++i)
		{
			float y = area.getBottom() - i * area.getHeight() / numYTicks;
			g.drawHorizontalLine((int)y, area.getX(), area.getRight());
		}

		// Vertical lines (log-freq)
		const std::vector<float> freqs{ 20,50,100,200,500,1000,2000,5000,10000,20000 };
		for (float freq : freqs)
		{
			if (freq < minFrequency || freq > maxFrequency)
				continue;

			float normX = (std::log10(freq) - logMin) / (logMax - logMin);
			float x = area.getX() + normX * area.getWidth();
			g.drawVerticalLine((int)x, area.getY(), area.getBottom());
		}
	}

	void drawAxes(juce::Graphics& g, juce::Rectangle<float> area)
	{
		g.setColour(juce::Colours::white);
		g.setFont(10.0f);

		// ==== Y AXIS (dB) ====
		constexpr int numYTicks = 6;
		for (int i = 0; i <= numYTicks; ++i)
		{
			float norm = (float)i / numYTicks;
			float y = area.getBottom() - norm * area.getHeight();

			float dbValue = juce::jmap(norm, 0.0f, 1.0f,
				minMagnitudeDb, maxMagnitudeDb);

			g.drawText(juce::String((int)dbValue) + " dB",
				2, (int)y - 7, 45, 14,
				juce::Justification::centred);
		}

		// ==== X AXIS (log-frequency) ====
		const std::vector<float> freqs{ 20,50,100,200,500,1000,2000,5000,10000,20000 };
		for (float freq : freqs)
		{
			if (freq < minFrequency || freq > maxFrequency)
				continue;

			float normX = (std::log10(freq) - logMin) / (logMax - logMin);
			float x = area.getX() + normX * area.getWidth();

			juce::String label = freq < 1000
				? juce::String((int)freq) + " Hz"
				: juce::String(freq / 1000.0f, 1) + " kHz";

			g.drawText(label,
				(int)x - 30,
				area.getBottom() + 5,
				60, 20,
				juce::Justification::centred);
		}
	}

	void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area)
	{
		juce::Path path;
		const int numBins = m_spectrum.size();

		auto mapX = [area, this](int bin)
		{
			float freq = (float)bin * (float)sampleRate / (float)fftSize;
			if (freq < minFrequency)
				freq = minFrequency;

			float normX = (std::log10(freq) - logMin) / (logMax - logMin);
			return area.getX() + normX * area.getWidth();
		};

		auto mapY = [area, this](float gainA)
		{
			float db = juce::Decibels::gainToDecibels(gainA);
			db = juce::jlimit(minMagnitudeDb, maxMagnitudeDb, db);

			float norm = juce::jmap(db, minMagnitudeDb, maxMagnitudeDb, 0.0f, 1.0f);
			return area.getBottom() - norm * area.getHeight();
		};

		path.startNewSubPath(mapX(0), mapY(m_spectrum[0]));

		for (int i = 1; i < numBins; ++i)
		{
			const float x = mapX(i);
			const float y = mapY(m_spectrum[i]);
			path.lineTo(x , y);
		}

		g.setColour(juce::Colours::red);
		g.strokePath(path, juce::PathStrokeType(1.0f));
	}

	//==============================================================================
	std::vector<float> m_spectrum;

	double sampleRate = 48000.0;
	int fftSize = 256; // must match your SpectrumMatchFFTDetect

	float minMagnitudeDb = -24.0f;
	float maxMagnitudeDb = 24.0f;

	float minFrequency = 20.0f;
	float maxFrequency = 20000.0f;
	float logMin = std::log10(20.0f);
	float logMax = std::log10(20000.0f);

	float m_ammount = 1.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumDifferenceComponent)
};
