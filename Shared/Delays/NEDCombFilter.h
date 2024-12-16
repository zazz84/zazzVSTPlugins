// https://www.researchgate.net/publication/2579199_Digital_Signal_Processing_Techniques_for_Non-exponentially_Decaying_Reverberation

#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class NEDCombFilter
{
public:
	NEDCombFilter() {};

	inline void init(const int size)
	{
		m_buffer1.init((20 + 1) * size);
		m_buffer2.init(size);
		m_buffer3.init(size);
	};
	inline void set(const int feedback, const int size)
	{
		if (size != m_size || feedback != m_feedback)
		{
			m_buffer1.release();
			m_buffer2.release();
			m_buffer3.release();

			m_size = size;
			m_feedback = feedback;
		}

		m_gain0 = 1.0f + 1.0f / (float)feedback;
		m_gain1 = m_gain0 - 1.0f;
		m_gain3 = 1.0f - m_gain1;
		m_sizeMultiplier = feedback + 1;
		
		m_buffer1.set(m_sizeMultiplier * size);
		m_buffer2.set(size);
		m_buffer3.set(size);
	};
	inline float process(const float in)
	{		
		const float delayOut0 = m_buffer1.readDelay(m_size);
		const float delayOut1 = in - m_gain0 * delayOut0 + m_gain1 * m_buffer1.read();
		m_buffer1.write(in);

		const float delayOut2 = m_buffer2.read();
		m_buffer2.write(delayOut1 + delayOut2);
		
		const float delayOut3 = m_buffer3.read();
		m_buffer3.write(delayOut2 + delayOut3);
		
		return m_gain3 * delayOut3 + delayOut0;
	};

private:
	CircularBuffer m_buffer1, m_buffer2, m_buffer3;

	float m_gain0 = 0.0f;
	float m_gain1 = 0.0f;
	float m_gain3 = 0.0f;
	int m_feedback = 0;
	int m_size = 0;
	int m_sizeMultiplier = 0;
};