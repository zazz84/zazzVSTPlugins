#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class Allpass
{
public:
	Allpass();

	inline void init(const int size)
	{ 
		m_buffer.init(size);
	};
	inline void set(const float feedback, const int size) 
	{ 
		m_feedback = feedback;
		m_buffer.setSize(size);
	};
	float process(const float in)
	{
		const float inAtt = (1.0f - m_buffer.getSize() * 0.0001f) * in;

		const float delayOut = m_buffer.read();
		m_buffer.write(inAtt - m_feedback * delayOut);

		return delayOut + m_feedback * inAtt;
	};

private:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};