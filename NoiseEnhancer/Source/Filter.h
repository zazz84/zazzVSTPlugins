/*
  ==============================================================================

    Filter.h
    Created: 30 Dec 2023 10:04:46am
    Author:  zazz

  ==============================================================================
*/

#pragma once

#include <math.h>

//==============================================================================
/**
*/
class TwoPoleBandPass
{
public:
	TwoPoleBandPass();

	void init(int sampleRate);
	void setCoef(float frequency, float resonance);
	float process(float in);

protected:
	int m_sampleRate;

	float m_a0 = 1.0f;
	float m_a1 = 0.0f;
	float m_a2 = 0.0f;
	float m_b0 = 0.0f;
	float m_b1 = 0.0f;
	float m_b2 = 1.0f;

	float m_x1 = 0.0f;
	float m_x2 = 0.0f;
	float m_y1 = 0.0f;
	float m_y2 = 0.0f;
};

//==============================================================================
class LowPassFilter
{
public:
	LowPassFilter() {};

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float frequency)
	{
		float warp = tanf((frequency * 3.141593f) / m_SampleRate);
		m_OutLastCoef = (1 - warp) / (1 + warp);
		m_InCoef = warp / (1 + warp);
	}
	float process(float in)
	{
		m_OutLast = m_InCoef * (in + m_InLast) + m_OutLastCoef * m_OutLast;
		m_InLast = in;
		return m_OutLast;
	}

protected:
	int   m_SampleRate = 48000;
	float m_InCoef = 1.0f;
	float m_OutLastCoef = 0.0f;

	float m_OutLast = 0.0f;
	float m_InLast = 0.0f;
};

//==============================================================================
class LadderFilter
{
public:
	void init(int sampleRate)
	{
		m_SampleRate = sampleRate;
		m_lowPassFilter[0].init(sampleRate);
		m_lowPassFilter[1].init(sampleRate);
		m_lowPassFilter[2].init(sampleRate);
		m_lowPassFilter[3].init(sampleRate);
	}
	void setCoef(float frequency)
	{
		m_lowPassFilter[0].setCoef(frequency);
		m_lowPassFilter[1].setCoef(frequency);
		m_lowPassFilter[2].setCoef(frequency);
		m_lowPassFilter[3].setCoef(frequency);
	}
	void setResonance(float resonance)
	{
		m_resonance = resonance;
	}
	float process(float in)
	{
		float lowPass = in - m_resonance * m_OutLast;

		lowPass = m_lowPassFilter[0].process(lowPass);
		lowPass = m_lowPassFilter[1].process(lowPass);
		lowPass = m_lowPassFilter[2].process(lowPass);
		lowPass = m_lowPassFilter[3].process(lowPass);

		m_OutLast = lowPass;
		return lowPass;
	}

protected:
	LowPassFilter m_lowPassFilter[4] = {};
	int   m_SampleRate = 48000;

	float m_OutLast = 0.0f;
	float m_resonance = 0.0f;
};

//==============================================================================
class LowPassFilter12dB : public LowPassFilter
{
public:
	float process(float in)
	{
		const float out1 = m_InCoef * (in + m_InLast) + m_OutLastCoef * m_OutLast;
		m_OutLast2 = m_InCoef * (out1 + m_OutLast) + m_OutLastCoef * m_OutLast2;

		m_InLast = in;
		m_OutLast = out1;
		return m_OutLast2;
	}

protected:
	float m_OutLast2 = 0.0f;
};

//==============================================================================
class HighPassFilter
{
public:
	HighPassFilter() {};

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float frequency)
	{
		const float x = exp(-2.0f * 3.141593f * frequency / m_SampleRate);
		m_InCoef = 1.0f - x;
		m_OutLastCoef = -x;
	}
	float process(float in)
	{
		m_OutLast = m_InCoef * in - m_OutLastCoef * m_OutLast;
		return in - m_OutLast;
	}

protected:
	int   m_SampleRate = 48000;
	float m_InCoef = 1.0f;
	float m_OutLastCoef = 0.0f;

	float m_OutLast = 0.0f;
};