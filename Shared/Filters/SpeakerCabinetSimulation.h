#pragma once#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"

class SpeakerCabineSimulation
{
public:
	SpeakerCabineSimulation() {};

	inline void init(const int sampleRate)
	{
		m_HPFilter.init(sampleRate);
		m_peakFilter1.init(sampleRate);
		m_peakFilter2.init(sampleRate);
		m_LPFilter.init(sampleRate);

		m_HPFilter.setHighPass(100.0f, 0.5f);
		m_peakFilter1.setPeak(230.0f, 0.5f, 8.0f);
		m_peakFilter2.setPeak(1000.0f, 2.5f, -7.0f);
		m_LPFilter.set(3800.0f);
	}
	inline void set()
	{
	}
	inline float process(const float in)
	{
		return m_HPFilter.processDF1(m_peakFilter1.processDF1(m_peakFilter2.processDF1(m_LPFilter.process(in))));
	}

private:
	BiquadFilter m_HPFilter, m_peakFilter1, m_peakFilter2;
	ForthOrderLowPassFilter m_LPFilter;
};