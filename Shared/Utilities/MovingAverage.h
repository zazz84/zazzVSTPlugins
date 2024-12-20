#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class MovingAverage
{
public:
	MovingAverage() {};
	
	inline void init(const int size)
	{
		m_buffer.init(size);
		m_size = (float)size;
	}
	inline void set(const float size)
	{
		m_size = size;
	}
	inline float process(const float in)
	{	
		m_sum -= m_buffer.readDelayLinearInterpolation(m_size);
		m_sum += in;
		m_buffer.write(in);

		return m_sum / (float)m_size;
	}

private:
	CircularBuffer m_buffer;
	float m_size = 0;
	float m_sum = 0.0f;
};