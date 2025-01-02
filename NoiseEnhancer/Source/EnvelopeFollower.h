/*
  ==============================================================================

    EnvelopeFollower.h
    Created: 30 Dec 2023 9:58:45am
    Author:  zazz

  ==============================================================================
*/

#pragma once

//==============================================================================
/**
*/
class EnvelopeFollower
{
public:
	EnvelopeFollower();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTime, float releaseTime);
	float process(float in);

protected:
	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_One_Minus_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;
	float m_One_Minus_ReleaseCoef = 0.0f;

	float m_OutLast = 0.0f;
	float m_Out1Last = 0.0f;
};