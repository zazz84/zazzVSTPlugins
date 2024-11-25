#pragma once

#include <random>

//==============================================================================
// Returns random number in range (0, 1)
class Random01
{
public:
	Random01() {};

	inline float process()
	{
		constexpr float normalize = 1.0f / (float)RAND_MAX;
		return (float)rand() * normalize;
	};
};

//==============================================================================
// Returns random number in range (0, 1)
class LehmerRandom01
{
public:
	LehmerRandom01() {};

	inline float process()
	{
		constexpr float normalize = 1.0f / 16777216.0f;
		m_outLast = (m_a * m_outLast) % m_m ;
		return (float)m_outLast / normalize;
	};

private:
	long int m_a = 5;
	long int m_m = 16777216;
	long int m_outLast = 3;
};

//==============================================================================
// Returns random number in range (0, 1)
class LinearCongruentialRandom01
{
public:
	LinearCongruentialRandom01() {};

	inline float process()
	{
		m_outLast = m_a * m_outLast + m_c;		
		return (float)((m_outLast >> 16) & 0x7fff) / 0x7fff;
	};
	inline void set(long seed)
	{
		m_outLast = seed;
	}

private:
	long m_m = 65537L;
	long m_a = 214013L;
	long m_c = 2531011L;
	long m_outLast = 79L;
};

//==============================================================================
// Returns random number in range (0, 1)
class LaggedFibonacciRandom01
{
public:
	LaggedFibonacciRandom01() {};

	inline float process()
	{
		int out = engine();
		constexpr float normalize = 1.0f / 16777216.0f;
		return (float)out * normalize;
	};

private:
	std::subtract_with_carry_engine<unsigned int, 24, 24, 55> engine;
};