#pragma once

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

	inline float processDF1(const float in)
	{
		const float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

		x2 = x1;
		x1 = in;

		y2 = y1;
		y1 = out;

		return out;
	}
	float processDF2(float in);
	float processDF1T(float in);
	inline float processDF2T(const float in)
	{
		const float out = b0 * in + x2;

		/*x2 = b1 * in + x1 + a1 * out;
		x1 = b2 * in + a2 * out;*/

		x2 = b1 * in + x1 - a1 * out;
		x1 = b2 * in - a2 * out;

		return out;
	}

	inline void release()
	{
		a0 = 0.0f;
		a1 = 0.0f;
		a2 = 0.0f;

		b0 = 0.0f;
		b1 = 0.0f;
		b2 = 0.0f;

		x1 = 0.0f;
		x2 = 0.0f;

		y1 = 0.0f;
		y2 = 0.0f;

		m_SampleRate = 48000;
	}

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