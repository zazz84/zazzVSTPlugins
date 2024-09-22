#pragma once
#include <algorithm>

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
template <class T>
class BaseEnvelopeFollower
{
public:
	BaseEnvelopeFollower() {};

	void init(int sampleRate)
	{
		m_SampleRate = sampleRate;
	}
	void setCoef(T attackTimeMs, T releaseTimeMs)
	{
		m_AttackCoef = exp(-1000.0 / (attackTimeMs * (T)m_SampleRate));
		m_ReleaseCoef = exp(-1000.0 / (releaseTimeMs * (T)m_SampleRate));
	};

protected:
	int  m_SampleRate = 48000;
	T m_AttackCoef = 0.0;
	T m_ReleaseCoef = 0.0;
	T m_OutLast = 0.0;
};

//==============================================================================
template <class T>
class BranchingEnvelopeFollower : public BaseEnvelopeFollower<T>
{
public:
	BranchingEnvelopeFollower() {};

	T process(T in)
	{
		const auto inAbs = abs(in);
		const auto coef = (inAbs > m_OutLast) ? m_AttackCoef : m_ReleaseCoef;

		return m_OutLast = inAbs + coef * (m_OutLast - inAbs);
	};
};

//==============================================================================
template <class T>
class DecoupeledEnvelopeFollower : public BaseEnvelopeFollower<T>
{
public:
	DecoupeledEnvelopeFollower() {};

	T process(T in)
	{
		const T inAbs = abs(in);
		m_OutReleaseLast = std::max(inAbs, inAbs + m_ReleaseCoef * (m_OutReleaseLast - inAbs));
		
		return m_OutLast = m_OutReleaseLast + m_AttackCoef * (m_OutLast - m_OutReleaseLast);
	};

private:
	T m_OutReleaseLast = 0.0;
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

//==============================================================================
class GainCompensation
{
public:
	GainCompensation() {};

	void init(int sampleRate)
	{
		m_inputEnvelopeFollower.init(sampleRate);
		m_outputEnvelopeFollower.init(sampleRate);

		m_inputEnvelopeFollower.setCoef(5.0f, 15.0f);
		m_outputEnvelopeFollower.setCoef(5.0f, 15.0f);
	}
	void set(float dynamics) { m_dynamics = dynamics; };
	float getGainCompensation(float in, float out);

private:
	EnvelopeFollower m_inputEnvelopeFollower;
	EnvelopeFollower m_outputEnvelopeFollower;

	float m_dynamics = 0.0f;
};