#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CombFilter.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/AllPassFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

//==============================================================================
class NestedCombFilter
public:
	NestedCombFilter();

	void init(int combFilterSize, int allPassSize, int sampleRate)
	{
		m_combFilter.init(combFilterSize);
		m_allPass.init(allPassSize);
		m_lowPassFilter.init(sampleRate);
	};
	void setSize(int combFilterSize, int allPassSize)
	{
		m_buffer.set(combFilterSize);
		m_allPass.setSize(allPassSize);
	};
	void setFeedback(float combFilterFeedback, float allPassFeedback)
	{
		m_feedback = combFilterFeedback;
		m_allPass.setFeedback(allPassFeedback);
	};
	void setDamping(float damping)
	{
		m_filter.setHighShelf(2000.0f, 0.7f, damping * -12.0f);
	};

	float NestedCombFilter::process(float in)
	{
		const float delayOut = m_buffer.read();
		m_buffer.write(in - m_allPassFilter.process(m_lowPassFilter.processDF1(m_feedback * delayOut)));
		return delayOut;
	}

private:
	CombFilter m_combFilter;
	AllPassFilter m_allPassFilter;
	BiquadFilter m_lowPassFilter;
};