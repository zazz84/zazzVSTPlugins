#include "Convolutions.h"
#include <string.h>

Convolution::Convolution()
{
}

void Convolution::init(int size)
{
	m_buffer.init(size);
	m_bufferSize = size;
}

void Convolution::clear()
{
	m_buffer.clear();
}

void Convolution::setImpulseResponse(std::vector<float> impulseResponse)
{
	m_impulseResponse = impulseResponse;
}

float Convolution::process(float in)
{
	m_buffer.writeSample(in);

	float out = 0.0f;
	for (int i = 0; i < m_impulseResponseSize; i++)
	{
		out += m_buffer.readDelay(m_impulseResponseSize - i) * m_impulseResponse[i];
	}

	return out;
}