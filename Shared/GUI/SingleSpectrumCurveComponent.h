#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/GUI/BaseSpectrumCurveComponent.h"

/**
	SpectrumDifferenceComponent
	Draws a logarithmic-frequency, dB-scaled spectrum.
	Input: linear FFT gains from SpectrumMatchFFTDetect
*/
class SingleSpectrumCurveComponent : public BaseSpectrumCurveComponent
{
public:
	SingleSpectrumCurveComponent() = default;
	~SingleSpectrumCurveComponent() = default;

	void setSpectrum(const std::vector<float>& spectrum)
	{
		m_spectrum.resize(spectrum.size());
		std::copy(spectrum.begin(), spectrum.end(), m_spectrum.begin());
		repaint();
	}

protected:
	void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area) override
	{
		if (m_spectrum.empty())
		{
			return;
		}
		
		juce::Path path;
		path.startNewSubPath(mapX(m_firstBin, area), mapY(m_spectrum[m_firstBin], area));

		for (int i = m_firstBin + 1; i < m_lastBin; ++i)
		{
			const float x = mapX(i, area);
			const float y = mapY(m_spectrum[i], area);
			path.lineTo(x, y);
		}

		g.setColour(juce::Colours::red);
		g.strokePath(path, juce::PathStrokeType(1.0f));
	}

private:
	std::vector<float> m_spectrum;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingleSpectrumCurveComponent)
};
