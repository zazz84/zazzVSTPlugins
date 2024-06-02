#include "EnvelopeFollowers.h"
#include <cmath>

constexpr auto PI = 3.14159265358979323846;

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