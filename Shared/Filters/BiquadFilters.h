#pragma once

//==============================================================================
class  BiquadFilter
{
public:
	BiquadFilter() {};

	inline void init(const int sampleRate)
	{ 
		m_SampleRate = sampleRate;
	}
	
	void setLowPass(float frequency, float Q);
	void setHighPass(float frequency, float Q);
	void setBandPassSkirtGain(float frequency, float Q);
	void setBandPassPeakGain(float frequency, float Q);
	void setNotch(float frequency, float Q);
	void setPeak(float frequency, float Q, float gain);
	void setLowShelf(float frequency, float Q, float gain);
	void setHighShelf(float frequency, float Q, float gain);

	float processDF1(float in);
	float processDF2(float in);
	float processDF1T(float in);
	float processDF2T(float in);

private:
	void normalize();
	
	float a0 = 0.0f;
	float a1 = 0.0f;
	float a2 = 0.0f;
	
	float b0 = 0.0f;
	float b1 = 0.0f;
	float b2 = 0.0f;
	
	float x1 = 0.0f;
	float x2 = 0.0f;

	float y1 = 0.0f;
	float y2 = 0.0f;

	int m_SampleRate = 48000;
};

//==============================================================================
class LinkwitzRileySecondOrder
{
public:
	LinkwitzRileySecondOrder() {};

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	};
	void setFrequency(float frequency);
	float processLP(float in);
	float processHP(float in);

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