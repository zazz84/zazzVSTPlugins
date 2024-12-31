#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

#include <JuceHeader.h>

class RMS
{
public:
	RMS() = default;
	~RMS()
	{
		release();
	}

	inline void init(const int size)
	{
		m_buffer.init(size);
		m_size = size;
	}
	inline float process(const float in)
	{
		juce::ScopedNoDenormals noDenormals;
		
		const float inAbs = Math::fabsf(in);
		
		m_sum -= m_buffer.read();
		m_sum += inAbs;

		m_buffer.write(inAbs);
				
		return m_sum / static_cast<float>(m_size);
	};
	inline void release()
	{
		m_buffer.release();
		m_sum = 0.0f;
		m_size = 0;
	}

private:
	CircularBuffer m_buffer;
	float m_sum = 0.0f;
	int m_size = 0;
};