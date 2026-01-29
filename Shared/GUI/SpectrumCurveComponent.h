/*#pragma once
#include <JuceHeader.h>

class SpectrumCurveComponent : public juce::Component
{
public:
	SpectrumCurveComponent() = default;

	//==============================================================================
	void setSpectrum(const float* newData, int numBins)
	{
		jassert(newData != nullptr);
		jassert(numBins > 0);

		spectrumData.assign(newData, newData + numBins);
		repaint();
	}

	void setSampleRate(double newSampleRate)
	{
		sampleRate = newSampleRate;
	}

	void setMagnitudeRangeDb(float minDb, float maxDb)
	{
		minMagnitudeDb = minDb;
		maxMagnitudeDb = maxDb;
	}

	//==============================================================================
	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colours::black);

		if (spectrumData.empty())
			return;

		constexpr int leftMargin = 50;
		constexpr int bottomMargin = 30;
		constexpr int topMargin = 10;
		constexpr int rightMargin = 10;

		auto plotArea = getLocalBounds()
			.reduced(rightMargin, topMargin)
			.withTrimmedLeft(leftMargin)
			.withTrimmedBottom(bottomMargin)
			.toFloat();

		drawGrid(g, plotArea);
		drawAxes(g, plotArea);
		drawSpectrum(g, plotArea);
	}

private:
	//==============================================================================
	void drawGrid(juce::Graphics& g, juce::Rectangle<float> area)
	{
		g.setColour(juce::Colours::darkgrey);

		for (int i = 1; i < 5; ++i)
		{
			float y = area.getY() + area.getHeight() * i / 5.0f;
			g.drawHorizontalLine((int)y, area.getX(), area.getRight());
		}

		for (int i = 1; i < 5; ++i)
		{
			float x = area.getX() + area.getWidth() * i / 5.0f;
			g.drawVerticalLine((int)x, area.getY(), area.getBottom());
		}
	}

	//==============================================================================
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
				minMagnitudeDb, maxMagnitudeDb);

			g.drawText(juce::String((int)dbValue) + " dB",
				2, (int)y - 7, 45, 14,
				juce::Justification::right);
		}

		// ==== X AXIS (Frequency) ====
		constexpr int numXTicks = 5;
		const int numBins = (int)spectrumData.size();

		for (int i = 0; i <= numXTicks; ++i)
		{
			float norm = (float)i / numXTicks;
			float x = area.getX() + norm * area.getWidth();

			float freq = sampleRate > 0.0
				? (float)(norm * 0.5 * sampleRate)
				: norm * (float)numBins;

			g.drawText(juce::String((int)freq) + " Hz",
				(int)x - 30,
				area.getBottom() + 5,
				60, 20,
				juce::Justification::centred);
		}
	}

	//==============================================================================
	void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area)
	{
		juce::Path path;
		const int numBins = (int)spectrumData.size();

		auto mapX = [area, numBins](int i)
		{
			return area.getX()
				+ area.getWidth() * (float)i / (float)(numBins - 1);
		};

		auto mapY = [area, this](float gain)
		{
			// Prevent -inf dB
			gain = juce::jmax(gain, 1.0e-6f);

			float db = juce::Decibels::gainToDecibels(gain);

			db = juce::jlimit(minMagnitudeDb, maxMagnitudeDb, db);

			float norm = juce::jmap(db,
				minMagnitudeDb, maxMagnitudeDb,
				0.0f, 1.0f);

			return area.getBottom() - norm * area.getHeight();
		};

		path.startNewSubPath(mapX(0), mapY(spectrumData[0]));

		for (int i = 1; i < numBins; ++i)
			path.lineTo(mapX(i), mapY(spectrumData[i]));

		g.setColour(juce::Colours::lime);
		g.strokePath(path, juce::PathStrokeType(2.0f));
	}

	//==============================================================================
	std::vector<float> spectrumData;

	double sampleRate = 48000.0;

	float minMagnitudeDb = -80.0f;
	float maxMagnitudeDb = 12.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumCurveComponent)
};*/

#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

/**
	SpectrumCurveComponent
	Draws a logarithmic-frequency, dB-scaled spectrum.
	Input: linear FFT gains from SpectrumMatchFFTDetect
*/
class SpectrumCurveComponent : public juce::Component
{
public:
	SpectrumCurveComponent() = default;

	//==============================================================================
	void setSpectrum(const float* newData, const int numBins, const int spectrumIndex = 0)
	{
		jassert(newData != nullptr);
		jassert(numBins > 0);

		spectrumData[spectrumIndex].assign(newData, newData + numBins);
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

		if (spectrumData[0].empty() && spectrumData[1].empty())
			return;

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
		drawSpectrum(g, plotArea);
	}

private:
	//==============================================================================
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
		constexpr int numYTicks = 5;
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
		const int numBins = std::max((int)spectrumData[0].size(), (int)spectrumData[1].size());

		auto mapX = [area, this](int bin)
		{
			float freq = (float)bin * (float)sampleRate / (float)fftSize;
			if (freq < minFrequency)
				freq = minFrequency;

			float normX = (std::log10(freq) - logMin) / (logMax - logMin);
			return area.getX() + normX * area.getWidth();
		};

		auto mapY = [area, this](float gain)
		{
			// Normalize / prevent log(0)
			gain = juce::jmax(gain, 1.0e-6f);

			float db = juce::Decibels::gainToDecibels(gain);
			db = juce::jlimit(minMagnitudeDb, maxMagnitudeDb, db);

			float norm = juce::jmap(db, minMagnitudeDb, maxMagnitudeDb, 0.0f, 1.0f);
			return area.getBottom() - norm * area.getHeight();
		};

		// Spectrum 0
		if (!spectrumData[0].empty())
		{
			path.startNewSubPath(mapX(0), mapY(spectrumData[0][0]));

			for (int i = 1; i < numBins; ++i)
				path.lineTo(mapX(i), mapY(spectrumData[0][i]));

			g.setColour(juce::Colours::yellow);
			g.strokePath(path, juce::PathStrokeType(1.0f));
		}
		
		// Spectrum 1
		if (!spectrumData[1].empty())
		{
			juce::Path path2;
			path2.startNewSubPath(mapX(0), mapY(spectrumData[1][0]));

			for (int i = 1; i < numBins; ++i)
			{
				const float x = mapX(i);
				//DBG(x);
				const float y = mapY(spectrumData[1][i]);

				path2.lineTo(x, y);
			}

			g.setColour(juce::Colours::white);
			g.strokePath(path2, juce::PathStrokeType(1.0f));
		}
	}

	//==============================================================================
	std::vector<float> spectrumData[2];

	double sampleRate = 48000.0;
	int fftSize = 256; // must match your SpectrumMatchFFTDetect

	float minMagnitudeDb = -80.0f;
	float maxMagnitudeDb = 12.0f;

	float minFrequency = 20.0f;
	float maxFrequency = 20000.0f;
	float logMin = std::log10(20.0f);
	float logMax = std::log10(20000.0f);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumCurveComponent)
};
