#include "CircularBuffers.h"
#include <math.h>
#include <string.h>

CircularBuffer::CircularBuffer()
{
}

void CircularBuffer::init(int size)
{
	m_head = 0;
	const int sizePowerOfTwo = GetPowerOfTwo(size);
	m_bitMask = sizePowerOfTwo - 1;

	m_readOffset = m_bitMask - size;
	
	m_buffer = new float[sizePowerOfTwo];
	memset(m_buffer, 0, sizePowerOfTwo * sizeof(float));
}

void CircularBuffer::clear()
{
	m_head = 0;
}

float CircularBuffer::readDelay(int sample)
{
	//const int readIdx = (m_head + m_bitMask - (int)(sample)) & m_bitMask;	
	const int readIdx = (m_head - 1 - sample) & m_bitMask;	

	return m_buffer[readIdx];
}

float CircularBuffer::readDelayLinearInterpolation(float sample)
{
	const int sampleTrunc = (int)(sample);

	const int readIdx = m_head + m_bitMask - sampleTrunc;
	const float weight = sample - sampleTrunc;
	
	const int iPrev = readIdx & m_bitMask;
	const int iNext = (readIdx + 1) & m_bitMask;

	return m_buffer[iNext] + weight * (m_buffer[iPrev] - m_buffer[iNext]);
}

float CircularBuffer::readDelayTriLinearInterpolation(float sample)
{
	const int sampleTrunc = (int)(sample);
	const int readIdx = m_head + m_bitMask - sampleTrunc;
	const float x = 1.0f - (sample - sampleTrunc);

	const int idx1 = (readIdx - 1) & m_bitMask;
	const int idx2 = readIdx & m_bitMask;
	const int idx3 = (readIdx + 1) & m_bitMask;
	const int idx4 = (readIdx + 2) & m_bitMask;

	const float yz1 = m_buffer[idx1];
	const float y0 = m_buffer[idx2];
	const float y1 = m_buffer[idx3];
	const float y2 = m_buffer[idx4];

	// 4-point, 2nd-order Watte tri-linear (x-form)
	float ym1py2 = yz1 + y2;
	float c0 = y0;
	float c1 = 3.0f / 2.0f * y1 - 1.0f / 2.0f * (y0 + ym1py2);
	float c2 = 1.0f / 2.0f * (ym1py2 - y0 - y1);

	return (c2 * x + c1) * x + c0;
}

float CircularBuffer::readDelayHermiteCubicInterpolation(float sample)
{
	const int sampleTrunc = (int)(sample);
	const int readIdx = m_head + m_bitMask - sampleTrunc;
	const float x = 1.0f - (sample - sampleTrunc);

	const int idx1 = (readIdx - 1) & m_bitMask;
	const int idx2 = readIdx & m_bitMask;
	const int idx3 = (readIdx + 1) & m_bitMask;
	const int idx4 = (readIdx + 2) & m_bitMask;

	const float yz1 = m_buffer[idx1];
	const float y0 = m_buffer[idx2];
	const float y1 = m_buffer[idx3];
	const float y2 = m_buffer[idx4];

	// 4-point, 3rd-order Hermite (x-form)
	float c0 = y0;
	float c1 = 1.0f / 2.0f * (y1 - yz1);
	float c2 = yz1 - 5.0f / 2.0f * y0 + 2.0f * y1 - 1.0f / 2.0f * y2;
	float c3 = 1.0f / 2.0f * (y2 - yz1) + 3.0f / 2.0f * (y0 - y1);
	
	return (( c3 * x + c2 ) * x + c1 ) * x + c0;
}

float CircularBuffer::readDelayOptimalCubicInterpolation(float sample)
{
	const int sampleTrunc = (int)(sample);
	const int readIdx = m_head + m_bitMask - sampleTrunc;
	const float x = 1.0f - (sample - sampleTrunc);

	const int idx1 = (readIdx - 1) & m_bitMask;
	const int idx2 = readIdx & m_bitMask;
	const int idx3 = (readIdx + 1) & m_bitMask;
	const int idx4 = (readIdx + 2) & m_bitMask;

	const float yz1 = m_buffer[idx1];
	const float y0 = m_buffer[idx2];
	const float y1 = m_buffer[idx3];
	const float y2 = m_buffer[idx4];

	// Optimal 2x (4-point, 3rd-order) (z-form)
	float z = x - 1.0f / 2.0f;
	float even1 = y1 + y0, odd1 = y1 - y0;
	float even2 = y2 + yz1, odd2 = y2 - yz1;
	float c0 = even1 * 0.45868970870461956f + even2 * 0.04131401926395584f;
	float c1 = odd1 * 0.48068024766578432f + odd2 * 0.17577925564495955f;
	float c2 = even1 * -0.246185007019907091f + even2 * 0.24614027139700284f;
	float c3 = odd1 * -0.36030925263849456f + odd2 * 0.10174985775982505f;
	
	return ((c3 * z + c2) * z + c1) * z + c0;
}

//==============================================================================
const float RoomEarlyReflections::delayTimesFactor[] = {
															0.3580f,
															0.4617f,
															0.5753f,
															0.5975f,
															0.9555f,
															0.7481f,
															1.0000f
};

const float RoomEarlyReflections::delayGains[] = {
													0.5968f,
													0.5228f,
													0.4540f,
													0.4421f,
													0.2996f,
													0.3718f,
													0.2871f
};

void RoomEarlyReflections::init(int size, int sampleRate)
{
	__super::init(size);

	m_sampleRate = sampleRate;

	for (auto filter : m_filter)
	{
		filter.init(sampleRate);
	}
}

float RoomEarlyReflections::process(float in)
{
	writeSample(in);
	
	const auto size = getSize();
	float out = 0.0f;

	for (int i = 0; i < N_DELAY_LINES; i++)
	{
		const float delayLineOut = delayGains[i] * readDelay((int)(size * delayTimesFactor[i]));
		out += m_filter[i].processDF1(delayLineOut);
	}

	return out;
}

//==============================================================================
float RMSBuffer::getRMS()
{
	const int size = getSize();

	m_rms = m_rms + ((fabsf(readDelay(1)) - fabsf(readDelay(size - 1))) / (float)size);

	// 1.5 is magic number
	return 1.5f * m_rms;
}