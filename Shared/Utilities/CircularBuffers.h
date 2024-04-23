class CircularBuffer
{
public:
	CircularBuffer();

	void init(int size);
	void clear();
	inline void setSize(int size) { m_readOffset = m_bitMask - size; };
	inline int getSize() { return m_bitMask - m_readOffset; };
	inline void writeSample(float sample)
	{
		m_buffer[m_head] = sample;
		m_head = (m_head + 1) & m_bitMask;
	}
	inline float read() const { return m_buffer[(m_head + m_readOffset) & m_bitMask]; };
	float readDelay(int sample);
	float readDelayLinearInterpolation(float sample);
	float readDelayTriLinearInterpolation(float sample);
	float readDelayHermiteCubicInterpolation(float sample);
	float readDelayOptimalCubicInterpolation(float sample);
	inline int GetPowerOfTwo(int i)
	{
		int n = i - 1;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n++;

		return n;
	}

protected:
	float *m_buffer;
	int m_head = 0;
	int m_bitMask = 0;
	int m_readOffset = 0;
};