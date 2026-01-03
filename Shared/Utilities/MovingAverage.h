#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class MovingAverage
{
public:
	MovingAverage() = default;

	~MovingAverage()
	{
		delete[] m_buffer;
	}

	inline void init(size_t size)
	{
		allocate(size);
	}

	// Only allocate when size changes
	inline void set(size_t size)
	{
		if (size != m_size)
			allocate(size);
		// else: do nothing, keep existing buffer and state
	}

	inline float process(float in)
	{
		// subtract oldest sample
		m_sum -= m_buffer[m_index];

		// write new sample
		m_buffer[m_index] = in;

		// add new sample to sum
		m_sum += in;

		// advance circular index
		m_index++;
		if (m_index >= m_size)
			m_index = 0;

		return m_sum * m_normalize;
	}

private:
	inline void allocate(size_t size)
	{
		delete[] m_buffer;
		m_buffer = new float[size];

		// zero buffer
		for (size_t i = 0; i < size; ++i)
			m_buffer[i] = 0.0f;

		m_size = size;
		m_index = 0;
		m_sum = 0.0f;
		m_normalize = 1.0f / float(size);
	}

private:
	float*  m_buffer = nullptr;
	size_t  m_size = 0;
	size_t  m_index = 0;
	float   m_sum = 0.0f;
	float   m_normalize = 0.0f;
};