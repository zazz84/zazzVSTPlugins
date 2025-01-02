/*
  ==============================================================================

    Filter.cpp
    Created: 30 Dec 2023 10:04:46am
    Author:  zazz

  ==============================================================================
*/

#include "Filter.h"

//==============================================================================
TwoPoleBandPass::TwoPoleBandPass()
{
}

void TwoPoleBandPass::init(int sampleRate)
{
	m_sampleRate = sampleRate;
}

void TwoPoleBandPass::setCoef(float frequency, float resonance)
{
	if (m_sampleRate == 0)
	{
		return;
	}

	const float omega = frequency * (2.0f * 3.141593f / m_sampleRate);
	const float sn = sinf(omega);
	const float alpha = sn * (1.0f - resonance);

	m_a0 = 1.0f + alpha;
	m_a1 = cosf(omega) * -2.0f;
	m_a2 = 1.0f - alpha;

	m_b0 = sn * 0.5f;
	m_b1 = 0.0f;
	m_b2 = -1.0f * m_b0;

	m_a1 /= m_a0;
	m_a2 /= m_a0;

	m_b0 /= m_a0;
	m_b1 /= m_a0;
	m_b2 /= m_a0;
}

float TwoPoleBandPass::process(float in)
{
	float y = m_b0 * in + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;

	m_y2 = m_y1;
	m_y1 = y;
	m_x2 = m_x1;
	m_x1 = in;

	return y;
}