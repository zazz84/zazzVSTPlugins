#pragma once

//#include <immintrin.h>

class  BiquadFilter
{
public:
	BiquadFilter() {};

	inline void init(int sampleRate) { m_SampleRate = sampleRate; }
	
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

	int m_SampleRate = 48000;
	
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
};

//==============================================================================
/*class  BiquadFilterSIMD
{
public:
	BiquadFilterSIMD() {};

	inline void init(int sampleRate) { m_SampleRate = sampleRate; }

	void setLowPass(float frequency, float Q);
	//void setHighPass(float frequency, float Q);
	//void setBandPassSkirtGain(float frequency, float Q);
	//void setBandPassPeakGain(float frequency, float Q);
	//void setNotch(float frequency, float Q);
	//void setPeak(float frequency, float Q, float gain);
	//void setLowShelf(float frequency, float Q, float gain);
	//void setHighShelf(float frequency, float Q, float gain);

	__m256 processDF1(__m256 in);
	//__m256 processDF2(__m256 in);
	//__m256 processDF1T(__m256 in);
	//__m256 processDF2T(__m256 in);

private:
	//void normalize();

	int m_SampleRate = 48000;

	__m256 m_x1, m_x2; // Input history
	__m256 m_y1, m_y2; // Output history
	__m256 m_b0, m_b1, m_b2, m_a1, m_a2; // Filter coefficients
};*/

//==============================================================================
class LinkwitzRileySecondOrder
{
public:
	LinkwitzRileySecondOrder();

	void init(int sampleRate);
	void setFrequency(float frequency);
	float processLP(float in);
	float processHP(float in);

protected:
	int m_SampleRate = 48000;
	
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
};