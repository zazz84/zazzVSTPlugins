#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/GUI/BaseSpectrumCurveComponent.h"

/**
	SpectrumCurveComponent
	Draws a logarithmic-frequency, dB-scaled spectrum.
	Input: linear FFT gains from SpectrumMatchFFTDetect
*/
class DualSpectrumCurveComponent : public BaseSpectrumCurveComponent
{
public:
	DualSpectrumCurveComponent() = default;
	~DualSpectrumCurveComponent() = default;

	void setSpectrum(const std::vector<float>& spectrum, const bool targetSpectrum = false)
	{		
		const int spectrumIndex = static_cast<int>(targetSpectrum);
		m_spectrum[spectrumIndex].resize(spectrum.size());
		std::copy(spectrum.begin(), spectrum.end(), m_spectrum[spectrumIndex].begin());
		repaint();
	}

protected:
	void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area) override
	{
		// Spectrum 0
		if (!m_spectrum[0].empty())
		{			
			juce::Path path1;
			path1.startNewSubPath(mapX(m_firstBin, area), mapY(m_spectrum[0][m_firstBin], area));

			for (int i = m_firstBin + 1; i < m_lastBin; i++)
			{
				const float x = mapX(i, area);
				const float y = mapY(m_spectrum[0][i], area);

				path1.lineTo(x, y);
			}

			g.setColour(juce::Colours::yellow);
			g.strokePath(path1, juce::PathStrokeType(1.0f));
		}
		
		// Spectrum 1
		if (!m_spectrum[1].empty())
		{
			juce::Path path2;
			path2.startNewSubPath(mapX(m_firstBin, area), mapY(m_spectrum[1][m_firstBin], area));

			for (int i = m_firstBin + 1; i < m_lastBin; i++)
			{
				const float x = mapX(i, area);
				const float y = mapY(m_spectrum[1][i], area);

				path2.lineTo(x, y);
			}

			g.setColour(juce::Colours::white);
			g.strokePath(path2, juce::PathStrokeType(1.0f));
		}
	}

private:
	std::vector<float> m_spectrum[2];

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualSpectrumCurveComponent)
};
