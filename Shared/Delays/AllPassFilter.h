#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class AllPassFilter
{
public:
	AllPassFilter() {};

	inline void init(const int size)
	{ 
		m_buffer.init(size);
	};
	inline void set(const float feedback, const int size) 
	{ 
		m_feedback = feedback;
		m_buffer.set(size);
	};
	float process(const float in)
	{
		const float delayOut = m_buffer.read();
		m_buffer.write(in - m_feedback * delayOut);

		return delayOut + m_feedback * in;
	};

private:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};