#pragma once

//==============================================================================
class EnvelopeFollower
{
public:
	EnvelopeFollower();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTimeMs, float releaseTimeMs);
	float process(float in);

protected:
	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;

	float m_OutLast = 0.0f;
	float m_Out1Last = 0.0f;
};

//==============================================================================
class SlewEnvelopeFollower
{
public:
	SlewEnvelopeFollower();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTimeMs, float releaseTimeMs);
	void setRange(float range);
	float process(float in);

protected:
	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;
	float m_Range = 1.0f;

	float m_OutLast = 0.0f;
};