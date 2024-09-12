#include "EnvelopeFollowers.h"
#include <cmath>

#define PI 3.14159265f

//==============================================================================
EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = exp(-1000.0f / (attackTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));
}

float EnvelopeFollower::process(float in)
{
	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
	return m_OutLast = m_AttackCoef * (m_OutLast - m_Out1Last) + m_Out1Last;
}

//==============================================================================
SlewEnvelopeFollower::SlewEnvelopeFollower()
{
}

void SlewEnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = 1000.0f / (attackTimeMs * m_SampleRate * m_Range);
	m_ReleaseCoef = 1000.0f / (releaseTimeMs * m_SampleRate * m_Range);
}

void SlewEnvelopeFollower::setRange(float range)
{
	m_Range = range;
}

float SlewEnvelopeFollower::process(float in)
{
	const float inAbs = fabs(in);
	//return m_OutLast = (inAbs > m_OutLast) ? m_OutLast + m_AttackCoef : m_OutLast - m_ReleaseCoef;

	float step = 0.0f;
	if (inAbs > m_OutLast)
	{
		step = fminf(inAbs - m_OutLast, m_AttackCoef);
		return m_OutLast += step;
	}
	else
	{
		step = fminf(m_OutLast - inAbs, m_ReleaseCoef);
		return m_OutLast -= step;
	}
}