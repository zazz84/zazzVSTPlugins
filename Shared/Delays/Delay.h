#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class Delay
{
public:
	Delay() {};

	inline void init(const int sampleRate, const int size)
	{
		m_buffer.init(size);

		m_sizeSmoother.init(sampleRate);
		m_sizeSmoother.set(2.0f);
	};
	inline void set(const float feedback, const int size)
	{
		m_size = size;
		m_feedback = feedback;
	};
	inline float process(const float in)
	{
		const int size = (int)m_sizeSmoother.process(m_size);
		const float delayOut = m_buffer.readDelay(size);
		m_buffer.write(in + m_feedback * delayOut);

		return delayOut;
	};

private:
	CircularBuffer m_buffer;
	OnePoleLowPassFilter m_sizeSmoother;
	float m_feedback = 0;
	int m_size = 0;
};