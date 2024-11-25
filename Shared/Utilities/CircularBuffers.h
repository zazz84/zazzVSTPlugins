#pragma once

#include <immintrin.h>
#include <vector>
#include <math.h>
#include <array>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

#define M_PI  3.14159268f
#define SPEED_OF_SOUND 343.0f

//==============================================================================
class CircularBuffer
{
public:
	CircularBuffer() {};

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
	/*void readSIMDFloat(const float* buffer, const size_t* indices, float* output, size_t count, size_t buffer_size) {
		for (size_t i = 0; i < count; i += 8) {
			// Load 8 indices
			__m256i offsets = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(indices + i));

			// Scalar fallback for modulo
			alignas(32) int mod_offsets[8];
			_mm256_store_si256(reinterpret_cast<__m256i*>(mod_offsets), offsets);

			for (int j = 0; j < 8; ++j) {
				mod_offsets[j] %= static_cast<int>(buffer_size); // Scalar modulo
			}

			// Load modulo results back into a SIMD register
			__m256i mod_offsets_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(mod_offsets));

			// Gather float values
			__m256 values = _mm256_i32gather_ps(buffer, mod_offsets_vec, 4);

			// Store results in the output array
			_mm256_storeu_ps(output + i, values);
		}
	}*/
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
	float* m_buffer;
	int m_head = 0;
	int m_bitMask = 0;
	int m_readOffset = 0;
};

//==============================================================================
class RMSBuffer : public CircularBuffer
{
public:
	float getRMS()
	{
		const int size = getSize();

		m_rms = m_rms + ((fabsf(readDelay(1)) - fabsf(readDelay(size - 1))) / (float)size);

		// 1.5 is magic number
		return 1.5f * m_rms;
	};

private:
	float m_rms = 0.0f;
};