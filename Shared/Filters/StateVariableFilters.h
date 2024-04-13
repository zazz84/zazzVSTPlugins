class  StateVariableFilter
{
public:
	StateVariableFilter() {};

	inline void init(int sampleRate) { m_SampleRate = sampleRate; }

	void setLowPass(float frequency, float Q);
	void setHighPass(float frequency, float Q);
	void setBandPass(float frequency, float Q);

	float processLowPass(float in);
	float processHighPass(float in);
	float processBandPass(float in);

private:
	int m_SampleRate = 48000;

	float a1 = 0.0f;
	float a2 = 0.0f;

	float b0 = 0.0f;
	float b1 = 0.0f;
	float b2 = 0.0f;

	float y1 = 0.0f;
	float y2 = 0.0f;
};