#include "CombFilters.h"

CombFilter::CombFilter()
{
}

float CombFilter::process(float in)
{
	const float delayOut = m_buffer.read();
	m_buffer.writeSample(in - m_feedback * delayOut);
	return delayOut;
}

//==============================================================================
Allpass::Allpass()
{
}

float Allpass::process(float in)
{
	const float delayOut = m_buffer.read();
	m_buffer.writeSample(in - m_feedback * delayOut);

	return delayOut + m_feedback * in;
}

//==============================================================================
NestedCombFilter::NestedCombFilter()
{
}

float NestedCombFilter::process(float in)
{
	const float delayOut = m_buffer.read();
	m_buffer.writeSample(in - m_allPass.process(m_filter.processDF1(m_feedback * delayOut)));
	return delayOut;
}

//==============================================================================
CircularCombFilter::CircularCombFilter()
{

}

void CircularCombFilter::init(int complexity, int* size)
{
	m_buffer = new CircularBuffer[complexity];
	m_feedback = new float[complexity];
	m_complexity = complexity;

	for (int i = 0; i < m_complexity; i++)
	{
		m_buffer[i].init(size[i]);
	}
}

void CircularCombFilter::setSize(const int* size)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_buffer[i].setSize(size[i]);
	}
}

void CircularCombFilter::setFeedback(const float* feedback)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_feedback[i] = feedback[i];
	}
}

float CircularCombFilter::process(float in)
{
	float out = 0.0;

	for (int i = 0; i < m_complexity; i++)
	{
		const float bufferOut = m_buffer[i].read();
		out += bufferOut;

		int writteIdx = i + 1;
		if (writteIdx >= m_complexity)
			writteIdx = 0;

		m_buffer[writteIdx].writeSample(in + m_feedback[writteIdx] * bufferOut);
	}

	return out;
}

//==============================================================================
CircularCombFilterAdvanced::CircularCombFilterAdvanced()
{

}

void CircularCombFilterAdvanced::init(int complexity, int* size, int* allPassSize)
{
	__super::init(complexity, size);

	m_filter = new BiquadFilter[complexity];
	m_allPass = new Allpass[complexity];

	for (int i = 0; i < m_complexity; i++)
	{
		m_allPass[i].init(allPassSize[i]);
		m_allPass[i].setFeedback(0.2f);
	}
}

void CircularCombFilterAdvanced::setDampingFrequency(const float* dampingFrequency)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_filter[i].setLowPass(dampingFrequency[i], 0.4f);
	}
}

void CircularCombFilterAdvanced::setAllPassSize(const int* size)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_allPass[i].setSize(size[i]);
	}
}

void CircularCombFilterAdvanced::setAllPassFeedback(const float* feedback)
{
	for (int i = 0; i < m_complexity; i++)
	{
		m_allPass[i].setFeedback(feedback[i]);
	}
}

float CircularCombFilterAdvanced::process(float in)
{
	float out = 0.0;

	for (int i = 0; i < m_complexity; i++)
	{
		const float bufferOut = m_filter[i].processDF1(m_allPass[i].process(m_buffer[i].read()));
		out += bufferOut;

		int writteIdx = i + 1;
		if (writteIdx >= m_complexity)
			writteIdx = 0;

		m_buffer[writteIdx].writeSample(in + m_feedback[writteIdx] * bufferOut);
	}

	return out;
}