#pragma once

#include <cmath>

class LinkwitzRileyFilter
{
public:
	LinkwitzRileyFilter() {};

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	};
	inline void set(const float frequency)
	{
		const float fpi = 3.141592f * frequency;
		const float wc = 2.0f * fpi;
		const float wc2 = wc * wc;
		const float wc22 = 2.0f * wc2;
		const float k = wc / tanf(fpi / m_SampleRate);
		const float k2 = k * k;
		const float k22 = 2 * k2;
		const float wck2 = 2 * wc * k;
		const float tmpk = k2 + wc2 + wck2;
		
		m_b1 = (-k22 + wc22) / tmpk;
		m_b2 = (-wck2 + k2 + wc2) / tmpk;
		
		//---------------
		// low-pass
		//---------------
		m_a0_lp = wc2 / tmpk;
		m_a1_lp = wc22 / tmpk;
		m_a2_lp = wc2 / tmpk;
		
		//----------------
		// high-pass
		//----------------
		m_a0_hp = k2 / tmpk;
		m_a1_hp = -k22 / tmpk;
		m_a2_hp = k2 / tmpk;
	};
	inline float processLP(const float in)
	{
		const float y0 = m_a0_lp * in + m_x0_lp;
		m_x0_lp = m_a1_lp * in - m_b1 * y0 + m_x1_lp;
		m_x1_lp = m_a2_lp * in - m_b2 * y0;

		return y0;
	}
	inline float processHP(const float in)
	{
		const float y0 = m_a0_hp * in + m_x0_hp;
		m_x0_hp = m_a1_hp * in - m_b1 * y0 + m_x1_hp;
		m_x1_hp = m_a2_hp * in - m_b2 * y0;

		return -y0;
	};

protected:	
	float m_b1 = 0.0f;
	float m_b2 = 0.0f;

	float m_a0_lp = 0.0f;
	float m_a1_lp = 0.0f;
	float m_a2_lp = 0.0f;

	float m_a0_hp = 0.0f;
	float m_a1_hp = 0.0f;
	float m_a2_hp = 0.0f;

	float m_x1_lp = 0.0f;
	float m_x0_lp = 0.0f;

	float m_x1_hp = 0.0f;
	float m_x0_hp = 0.0f;

	int m_SampleRate = 48000;
};