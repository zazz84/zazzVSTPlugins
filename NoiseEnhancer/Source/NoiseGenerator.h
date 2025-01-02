/*
  ==============================================================================

    NoiseGenerator.h
    Created: 30 Dec 2023 9:06:31pm
    Author:  zazz

  ==============================================================================
*/

#pragma once

#include <stdlib.h>
#include <math.h>
#include <random>

//==============================================================================
class RandomNoiseGenerator
{
public:
	RandomNoiseGenerator() {};

	float process()
	{
		return (2.0f * rand() / RAND_MAX) - 1.0f;
	};
};

//==============================================================================
class FastNoiseGenerator
{
public:
	FastNoiseGenerator() {};

	float process()
	{
		m_x1 ^= m_x2;
		const float out = m_x2 / 2147483647.0f;
		m_x2 += m_x1;
		return out;
	};

private:
	int m_x1 = 0x67452301;
	int m_x2 = 0xefcdab89;
};

//==============================================================================
class MiddleSquareNoiseGenerator
{
public:
	MiddleSquareNoiseGenerator() {};

	float process()
	{
		const double sq = (double)m_seed * (double)m_seed;

		m_seed = (int)(sq * 0.01f) % 10000;
		
		const float out = (2.0f * (float)m_seed * 0.0001f) - 1.0f;

		if (m_seed == 1600 ||
			m_seed == 9600 ||
			m_seed == 5600 ||
			m_seed == 3600 ||
			m_seed == 2916 ||
			m_seed == 540  ||
			m_seed == 5030 ||
			m_seed == 3009 ||
			m_seed == 4100 ||
			m_seed == 2100 ||
			m_seed == 8100 ||
			m_seed == 6100 ||
			m_seed == 0    ||
			m_seed == 100  ||
			m_seed == 2500 ||
			m_seed == 3792 ||
			m_seed == 6707 ||
			m_seed == 7600 ||
			m_seed == 6688)
		{
			m_seed = (float)(m_conter++);
			m_conter %= 10000;
		}

		return out;
	};

private:
	int m_seed = 0;
	int m_conter = 0;
};

//==============================================================================
class LehmerNoiseGenerator
{
public:
	LehmerNoiseGenerator() {};

	float process()
	{
		m_outLast = (m_a * m_outLast) % m_m ;
		return (2.0f * (float)m_outLast / (float)(m_m)) - 1.0f;
	};

private:
	long int m_a = 5;
	long int m_m = 16777216;
	long int m_outLast = 3;
};

//==============================================================================
class LinearCongruentialNoiseGenerator
{
public:
	LinearCongruentialNoiseGenerator() {};

	void setSeed(long seed)
	{
		m_outLast = seed;
	}
	float process()
	{
		m_outLast = m_a * m_outLast + m_c;
		
		return (2.0f * (float)((m_outLast >> 16) & 0x7fff) / 0x7fff) - 1.0f;
	};

private:
	long m_m = 65537L;
	long m_a = 214013L;
	long m_c = 2531011L;
	long m_outLast = 79L;
};

//==============================================================================
class LaggedFibonacciNoiseGenerator
{
public:
	LaggedFibonacciNoiseGenerator() {};

	float process()
	{
		int out = engine();
		return (2.0f * (float)(out) / 16777216.0f) - 1.0f;
	};

private:
	std::subtract_with_carry_engine<unsigned int, 24, 24, 55> engine;
};

//==============================================================================
class MersenneTwisterNoiseGenerator
{
public:
	MersenneTwisterNoiseGenerator() {};

	float process()
	{
		bool out = distribution(generator);
		return (2.0f * (float)out) - 1.0f;
	};

private:
	std::mt19937 generator{ 123 };
	std::uniform_real_distribution<float> distribution{ -1.0, 1.0 };
};

//==============================================================================
class WhiteNoiseGenerator
{
public:
	WhiteNoiseGenerator() {};

	enum DistributionType
	{
		Uniform,
		Normal,
		Bernoulli,
		PieceWise
	};

	void setDistributionType(DistributionType distributionType)
	{
		m_distributionType = distributionType;
	}
	float process()
	{
		if (m_distributionType == DistributionType::Uniform)
		{
			return m_realDistribution(m_mersenneTwisterGenerator);
		}
		else if (m_distributionType == DistributionType::Normal)
		{
			return m_normalDistribution(m_mersenneTwisterGenerator);
		}
		else if (m_distributionType == DistributionType::Bernoulli)
		{
			return (2.0f * m_bernoulliDistribution(m_mersenneTwisterGenerator)) - 1.0f;
		}
		else
		{
			return m_pieceWiceDistribution(m_mersenneTwisterGenerator);
		}
	}

private:
	DistributionType m_distributionType = DistributionType::Uniform;

	// Generators
	std::mt19937 m_mersenneTwisterGenerator{ 123 };

	// Distributions
	std::uniform_real_distribution<float> m_realDistribution{ -1.0, 1.0 };

	std::normal_distribution<float> m_normalDistribution{ 0.0f, 0.65f };

	std::vector<double> i{ -1.0, -0.1, 0.1, 1.0};
	std::vector<double> w{ 1, 30, 1 };
	std::piecewise_constant_distribution<> m_pieceWiceDistribution{ i.begin(), i.end(), w.begin() };

	std::bernoulli_distribution m_bernoulliDistribution{ 0.5 };
};