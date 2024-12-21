#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class CombFilter
{
public:
	CombFilter() {};

	inline void init(const int size)
	{ 
		m_buffer.init(size);
	};
	inline void set(const float feedback, const int size)
	{ 
		m_feedback = feedback;
		m_buffer.set(size);
	};
	inline float process(const float in)
	{
		const float delayOut = m_buffer.read();
		m_buffer.write(in - m_feedback * delayOut);
		return delayOut;
	};
	inline void release()
	{
		m_buffer.release();
		m_feedback = 0.0f;
	}
	inline float getSize() const
	{ 
		return m_buffer.getSize();
	};

protected:
	CircularBuffer m_buffer;
	float m_feedback = 0.0f;
};