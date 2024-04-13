#include "StateVariableFilters.h"
#include <cmath>

#define PI 3.14159265f

void StateVariableFilter::setLowPass(float frequency, float Q)
{
	const float w = 2.0f * tanf(PI * frequency / m_SampleRate);
	const float a = w / Q;
	const float b = w * w;
	
	const float tmp = a + b;
	a1 = tmp / (1.0f + 0.5f * a + 0.25f * b);
	a2 = b / tmp;

	b0 = 0.25f * a1 * a2;	b1 = 0.0f;
	b2 = 0.0f;
}

void StateVariableFilter::setHighPass(float frequency, float Q)
{
	const float w = 2.0f * tanf(PI * frequency / m_SampleRate);
	const float a = w / Q;
	const float b = w * w;
	
	const float tmp = a + b;
	a1 = tmp / (1.0f + 0.5f * a + 0.25f * b);
	a2 = b / tmp;

	b0 = 1.0f - 0.5f * a1 + 0.25f * a1 * a2;
	b1 = 0.0f;
	b2 = 0.0f;
}

void StateVariableFilter::setBandPass(float frequency, float Q)
{
	const float w = 2.0f * tanf(PI * frequency / m_SampleRate);
	const float a = w / Q;
	const float b = w * w;
	
	const float tmp = a + b;
	a1 = tmp / (1.0f + 0.5f * a + 0.25f * b);
	a2 = b / tmp;

	b1 = 1.0f - a2;
	b0 = 0.25f * b1 * a1;
	b2 = 0.0f;
}

float StateVariableFilter::processLowPass(float in)
{
	const float x = in - y1 - y2;
	y2 += a2 * y1;
	const float out = b0 * x + y2;
	y1 += a1 * x;	return out;
}

float StateVariableFilter::processHighPass(float in)
{
	const float x = in - y1 - y2;
	const float out = b0 * x;
	y2 += a2 * y1;
	y1 += a1 * x;
	return out;
}

float StateVariableFilter::processBandPass(float in)
{
	const float x = in - y1 - y2;
	const float out = b0 * x + b1 * y1;
	y2 += a2 * y1;
	y1 += a1 * x;

	return out;
}