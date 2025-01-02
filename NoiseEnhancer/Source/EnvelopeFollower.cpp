/*
  ==============================================================================

    EnvelopeFollower.cpp
    Created: 30 Dec 2023 9:58:45am
    Author:  zazz

  ==============================================================================
*/

#include "EnvelopeFollower.h"
#include <math.h>

//==============================================================================
EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = exp(-1000.0f / (attackTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));

	m_One_Minus_AttackCoef = 1.0f - m_AttackCoef;
	m_One_Minus_ReleaseCoef = 1.0f - m_ReleaseCoef;
}

float EnvelopeFollower::process(float in)
{
	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + m_One_Minus_ReleaseCoef * inAbs);
	return m_OutLast = m_AttackCoef * m_OutLast + m_One_Minus_AttackCoef * m_Out1Last;
}
