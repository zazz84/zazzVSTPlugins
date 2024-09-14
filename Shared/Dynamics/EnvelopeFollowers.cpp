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
	// Decoupeled
	/*m_Out1Last = fmax(fabs(in), m_ReleaseCoef * m_Out1Last);
	return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * m_Out1Last;*/

	// Branching
	/*const float inAbs = fabs(in);
	if (inAbs > m_OutLast)
	{
		return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * inAbs;
	}
	else
	{
		return m_OutLast = m_ReleaseCoef * m_OutLast;
	}*/

	// Decoupeled smooth
	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
	return m_OutLast = m_AttackCoef * (m_OutLast - m_Out1Last) + m_Out1Last;

	// Branching Smooth
	/*const float inAbs = fabs(in);
	if (inAbs > m_OutLast)
	{
		return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * inAbs;
	}
	else
	{
		return m_OutLast = m_ReleaseCoef * m_OutLast + (1.0f - m_ReleaseCoef) * inAbs;
	}*/
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

	if (inAbs > m_OutLast)
	{
		const float step = fminf(inAbs - m_OutLast, m_AttackCoef);
		return m_OutLast += step;
	}
	else
	{
		const float step = fminf(m_OutLast - inAbs, m_ReleaseCoef);
		return m_OutLast -= step;
	}
}

//==============================================================================
OptoEnvelopeFollower::OptoEnvelopeFollower()
{
}

void OptoEnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackTime = attackTimeMs;
	m_ReleaseTime = releaseTimeMs;
}

void OptoEnvelopeFollower::updateCoef()
{
	//const float attactTimeMs = m_AttackTime * (0.125613f + 0.873696f * expf(-0.2510894f * m_OutLast));
	//const float releaseTimeMs = m_ReleaseTime * (0.1259045f + 0.8728663f * expf(-0.2331243f * m_OutLast));

	// Simplified calculation
	// Attack and release time gets shorter with increasing gain reduction
	const float attactTimeMs = m_AttackTime * (3.0f / (m_OutLast + 3.0f));
	const float releaseTimeMs = m_ReleaseTime * (2.5f / (m_OutLast + 2.5f));

	m_AttackCoef = exp(-1000.0f / (attactTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));
}

float OptoEnvelopeFollower::process(float in)
{
	updateCoef();

	const float inAbs = fabs(in);
	m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
	return m_OutLast = m_AttackCoef * (m_OutLast - m_Out1Last) + m_Out1Last;
}

//==============================================================================
DualEnvelopeFollower::DualEnvelopeFollower()
{
}

float DualEnvelopeFollower::process(float in)
{
	float inAbs = fabs(in);

	float inAbsFilterFast = 0.0f;
	float inAbsFilterSlow = 0.0f;

	if (inAbs > m_Threshold)
	{
		inAbsFilterFast = inAbs - m_Threshold;
		inAbsFilterSlow = m_Threshold;
	}
	else
	{
		inAbsFilterFast = 0.0f;
		inAbsFilterSlow = inAbs;
	}

	// Adjust attacka and release time
	const float diff = fabsf(in - m_OutLast);
	const float timeFactor = 1.0f + 0.15f * (diff - 4.0f);
	m_FilterFast.setCoef(timeFactor * m_AttackTime, timeFactor * m_ReleaseTime);

	return m_OutLast = m_FilterFast.process(inAbsFilterFast) + m_FilterSlow.process(inAbsFilterSlow);
}