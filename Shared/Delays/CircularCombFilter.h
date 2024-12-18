#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

//==============================================================================
class CircularCombFilter
{
public:
	CircularCombFilter() {};

	inline void init(int complexity, int* size)
	{
		m_buffer = new CircularBuffer[complexity];
		m_feedback = new float[complexity];
		m_complexity = complexity;

		for (int i = 0; i < m_complexity; i++)
		{
			m_buffer[i].init(size[i]);
		}
	};
	void setSize(const float* feedback, const int* size, const int complexity)
	{
		m_complexity = complexity;
		
		for (int i = 0; i < complexity; i++)
		{
			m_feedback[i] = feedback[i];
			m_buffer[i].set(size[i]);
		}
	};
	inline float process(float in)
	{
		float out = 0.0;

		for (int i = 0; i < m_complexity; i++)
		{
			const float bufferOut = m_buffer[i].read();
			out += bufferOut;

			int writteIdx = i + 1;
			if (writteIdx >= m_complexity)
				writteIdx = 0;

			m_buffer[writteIdx].write(in + m_feedback[writteIdx] * bufferOut);
		}

		return out;
	}

protected:
	CircularBuffer* m_buffer;
	float* m_feedback;
	int m_complexity = 0;
};