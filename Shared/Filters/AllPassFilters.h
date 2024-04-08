#define PI 3.141592653589793f

//==============================================================================
class FirstOrderAllPass
{
public:
	FirstOrderAllPass();

	void init(int sampleRate);
	void setFrequency(float frequency);
	float process(float in);

protected:
	float m_SampleRate;
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
};

//==============================================================================
class SecondOrderAllPass
{
public:
	SecondOrderAllPass();

	void init(int sampleRate);
	void setFrequency(float frequency, float Q);
	float process(float in);

protected:
	float m_SampleRate;

	float m_xnz2 = 0.0f;
	float m_xnz1 = 0.0f;
	float m_ynz2 = 0.0f;
	float m_ynz1 = 0.0f;

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;
};