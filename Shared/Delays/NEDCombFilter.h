// https://www.researchgate.net/publication/2579199_Digital_Signal_Processing_Techniques_for_Non-exponentially_Decaying_Reverberation

#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class NEDCombFilter
{
public:
	NEDCombFilter() {};

	inline void init(const int size)
	{
		m_buffer1.init(size);
		m_buffer2.init(size);
		m_buffer3.init(size);

		m_sizeSmoother.init(48000);
		m_sizeSmoother.set(2.0f);
	};
	inline void set(const float feedback, const int size)
	{
		m_feedback = feedback;
		m_size = size;
		m_buffer1.set(size);
		m_buffer2.set(size);
		m_buffer3.set(size);
	};
	inline float process(const float in)
	{		
		const float size = m_sizeSmoother.process(m_size);
		
		const float delayOut1 = in - (1.0f - m_feedback) * m_buffer1.readDelayLinearInterpolation(size);
		m_buffer1.write(in);

		const float delayOut2 = m_buffer2.readDelayLinearInterpolation(size);
		m_buffer2.write(delayOut1 + m_feedback * delayOut2);
		
		const float delayOut3 = m_buffer3.readDelayLinearInterpolation(size);
		m_buffer3.write(delayOut2 + m_feedback * delayOut3);
		
		return delayOut3;
	};

private:
	CircularBuffer m_buffer1, m_buffer2, m_buffer3;
	OnePoleLowPassFilter m_sizeSmoother;

	float m_feedback = 0.0f;
	int m_size = 0;
};