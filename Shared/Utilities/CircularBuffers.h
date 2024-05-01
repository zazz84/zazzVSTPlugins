#pragma once

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

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

class RoomEarlyReflections : public CircularBuffer
{
public:
	RoomEarlyReflections() {};

	static const float delayTimesFactor[];
	static const float delayGains[];
	static const int N_DELAY_LINES = 7;

	void init(int size, int sampleRate);
	void setDamping(float damping) 
	{
		const float frequencyMax = m_sampleRate * 0.4f;
		
		for (int i = 0; i < N_DELAY_LINES; i++)
		{
			//const float frequency = fminf(frequencyMax, 18000.0f - delayTimesFactor[i] * 16000.0f);
			const float frequency = 18000.0f - damping * delayTimesFactor[i] * 17500.0f;
			m_filter[i].setLowPass(frequency, 0.7f);
		}
		
		m_damping = damping;
	};
	float process(float in);

private:
	float m_damping = 0.0f;
	int m_sampleRate = 48000;
	BiquadFilter m_filter[N_DELAY_LINES];
};