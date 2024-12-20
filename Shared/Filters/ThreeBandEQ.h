#pragma once

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/Filters/FirstOrderAllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/LinkwitzRileyFilter.h"

class ThreeBandEQ
{
public:
	ThreeBandEQ() {};

	inline void init(const int sampleRate)
	{
		m_lowMidFilter.init(sampleRate);
		m_midHighFilter.init(sampleRate);
		m_allPassFilter.init(sampleRate);
	};
	inline void set(const float frequencyLowMid, const float frequencyMidHigh, const float gainLowdB, const float gainMiddB, const float gainHighdB)
	{
		m_lowMidFilter.set(frequencyLowMid);
		m_midHighFilter.set(frequencyMidHigh);
		m_allPassFilter.set(frequencyMidHigh);

		m_gainLow = juce::Decibels::decibelsToGain(gainLowdB);
		m_gainMid = juce::Decibels::decibelsToGain(gainMiddB);
		m_gainHigh = juce::Decibels::decibelsToGain(gainHighdB);
	};
	inline float process(const float in)
	{
		const float low = m_lowMidFilter.processLP(in);
		const float midHigh = m_lowMidFilter.processHP(in);

		const float mid = m_midHighFilter.processLP(midHigh);
		const float high = m_midHighFilter.processHP(midHigh);

		const float lowAllPass = m_allPassFilter.process(low);

		return m_gainLow * lowAllPass + m_gainMid * mid + m_gainHigh * high;
	}

protected:	
	LinkwitzRileyFilter m_lowMidFilter;
	LinkwitzRileyFilter m_midHighFilter;
	FirstOrderAllPassFilter m_allPassFilter;

	float m_gainLow = 1.0f;
	float m_gainMid = 1.0f;
	float m_gainHigh = 1.0f;
};