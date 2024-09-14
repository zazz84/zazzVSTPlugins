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

//==============================================================================
class OptoEnvelopeFollower
{
public:
	OptoEnvelopeFollower();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTimeMs, float releaseTimeMs);
	float process(float in);

protected:
	void updateCoef();

	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;
	float m_AttackTime = 0.0f;
	float m_ReleaseTime = 0.0f;

	float m_OutLast = 0.0f;
	float m_Out1Last = 0.0f;
};

//==============================================================================
class DualEnvelopeFollower
{
public:
	DualEnvelopeFollower();

	void init(int sampleRate)
	{
		m_FilterFast.init(sampleRate);
		m_FilterSlow.init(sampleRate);
	}
	void setCoef(float attackTimeMs, float releaseTimeMs)
	{
		m_AttackTime = attackTimeMs;
		m_ReleaseTime = releaseTimeMs;
		
		m_FilterFast.setCoef(attackTimeMs, releaseTimeMs);
		m_FilterSlow.setCoef(150.0f, 600.0f);
	}
	void setThreshold(float threshold) { m_Threshold = threshold; }
	float process(float in);

protected:
	EnvelopeFollower m_FilterFast;
	EnvelopeFollower m_FilterSlow;
	float m_Threshold = 6.0f;

	float m_AttackTime = 0.0f;
	float m_ReleaseTime = 0.0f;

	float m_OutLast = 0.0f;
};