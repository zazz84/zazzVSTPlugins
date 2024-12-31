#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class ZeroCrossingRate
{
public:
	ZeroCrossingRate() = default;
	~ZeroCrossingRate() = default;

	inline void init(const int sampleRate)
	{
		m_hpFilter.init(sampleRate);
		m_lpFilter.init(sampleRate);

		m_hpFilter.setHighPass(40.0f, 0.707f);
		m_lpFilter.setHighPass(440.0f, 0.707f);
	};
	inline int process(const float in)
	{		
		const float inFiltered = m_hpFilter.processDF1(m_lpFilter.processDF1(in));
		
		if (m_inLast > 0.0f && inFiltered < 0.0f || m_inLast < 0.0f && inFiltered > 0.0f)
		{
			m_rateSamples = m_samplesSinceLast;
			m_samplesSinceLast = 0;
		}
		else
		{
			m_samplesSinceLast++;
		}

		m_inLast = inFiltered;

		return m_rateSamples;			// returns zero crossing rate in samples
	}

private:
	BiquadFilter m_hpFilter, m_lpFilter;
	float m_inLast = 0.0f;
	int m_samplesSinceLast = 0;
	int m_rateSamples = 0;
};