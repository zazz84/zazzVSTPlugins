#pragma once

#include <cmath>
#include <algorithm>
#include <type_traits>

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class BaseEnvelopeFollower
{
public:
	BaseEnvelopeFollower() = default;
	~BaseEnvelopeFollower() = default;

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	}
	void set(T attackTimeMs, T releaseTimeMs)
	{
		m_AttackCoef = exp(T(-1000.0) / (attackTimeMs * static_cast<T>(m_SampleRate)));
		m_ReleaseCoef = exp(T(-1000.0) / (releaseTimeMs * static_cast<T>(m_SampleRate)));
	};

protected:
	int  m_SampleRate = 48000;
	T m_AttackCoef = T(0.0);
	T m_ReleaseCoef = T(0.0);
	T m_OutLast = T(0.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class BranchingEnvelopeFollower : public BaseEnvelopeFollower<T>
{
public:
	BranchingEnvelopeFollower() = default;
	~BranchingEnvelopeFollower() = default;

	T process(T in)
	{
		const auto inAbs = std::abs(in);
		const auto coef = (inAbs > m_OutLast) ? m_AttackCoef : m_ReleaseCoef;

		return m_OutLast = inAbs + coef * (m_OutLast - inAbs);
	};
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class DecoupeledEnvelopeFollower : public BaseEnvelopeFollower<T>
{
public:
	DecoupeledEnvelopeFollower() = default;
	~DecoupeledEnvelopeFollower() = default;

	T process(T in)
	{
		const T inAbs = std::abs(in);
		m_OutReleaseLast = std::max(inAbs, inAbs + m_ReleaseCoef * (m_OutReleaseLast - inAbs));
		
		return m_OutLast = m_OutReleaseLast + m_AttackCoef * (m_OutLast - m_OutReleaseLast);
	};

private:
	T m_OutReleaseLast = T(0.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class SlewEnvelopeFollower
{
public:
	SlewEnvelopeFollower() = default;
	~SlewEnvelopeFollower() = default;

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	}
	inline void set(T attackTimeMs, T releaseTimeMs, T range = T(1.0))
	{
		m_AttackCoef = T(1000.0) / (attackTimeMs * static_cast<T>(m_SampleRate) * m_Range);
		m_ReleaseCoef = T(1000.0) / (releaseTimeMs * static_cast<T>(m_SampleRate) * m_Range);

		m_Range = range;
	}
	inline T process(const T in)
	{
		const T inAbs = std::abs(in);

		if (inAbs > m_OutLast)
		{
			const T step = std::min(inAbs - m_OutLast, m_AttackCoef);
			return m_OutLast += step;
		}
		else
		{
			const T step = std::min(m_OutLast - inAbs, m_ReleaseCoef);
			return m_OutLast -= step;
		}
	}

protected:
	int  m_SampleRate = 48000;
	T m_AttackCoef = T(0.0);
	T m_ReleaseCoef = T(0.0);
	T m_OutLast = T(0.0);
	T m_Range = T(1.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class OptoEnvelopeFollower
{
public:
	OptoEnvelopeFollower() = default;
	~OptoEnvelopeFollower() = default;

	inline void init(const int sampleRate)
	{
		m_SampleRate = sampleRate;
	}
	inline void set(T attackTimeMs, T releaseTimeMs)
	{
		m_AttackTime = attackTimeMs;
		m_ReleaseTime = releaseTimeMs;
	};
	inline T process(T in)
	{
		updateCoef();

		const T inAbs = std::abs(in);
		m_Out1Last = std::max(inAbs, m_ReleaseCoef * m_Out1Last + (static_cast<T>(1.0) - m_ReleaseCoef) * inAbs);
		
		return m_OutLast = m_AttackCoef * (m_OutLast - m_Out1Last) + m_Out1Last;
	};

protected:
	void updateCoef()
	{
		// Attack and release time gets shorter with increasing gain reduction
		const T attackTimeMs = m_AttackTime * (T(3.0) / (m_OutLast + T(3.0)));
		const T releaseTimeMs = m_ReleaseTime * (T(2.5) / (m_OutLast + T(2.5)));

		m_AttackCoef = exp(T(-1000.0) / (attackTimeMs * static_cast<T>(m_SampleRate)));
		m_ReleaseCoef = exp(T(-1000.0) / (releaseTimeMs * static_cast<T>(m_SampleRate)));
	};

	int  m_SampleRate = 48000;
	T m_AttackCoef = T(0.0);
	T m_ReleaseCoef = T(0.0);
	T m_AttackTime = T(0.0);
	T m_ReleaseTime = T(0.0);

	T m_OutLast = T(0.0);
	T m_Out1Last = T(0.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class DualEnvelopeFollower
{
public:
	DualEnvelopeFollower() = default;
	~DualEnvelopeFollower() = default;

	inline void init(const int sampleRate)
	{
		m_FilterFast.init(sampleRate);
		m_FilterSlow.init(sampleRate);
	};
	inline void setCoef(const T attackTimeMs, const T releaseTimeMs)
	{
		m_AttackTime = attackTimeMs;
		m_ReleaseTime = releaseTimeMs;
		
		m_FilterFast.set(attackTimeMs, releaseTimeMs);
		m_FilterSlow.set(static_cast<T>(150.0), static_cast<T>(600.0));
	};
	inline void setThreshold(const T threshold)
	{ 
		m_Threshold = threshold;
	};
	inline T process(const T in)
	{
		T inAbs = std::abs(in);

		T inAbsFilterFast = T(0.0);
		T inAbsFilterSlow = T(0.0);

		if (inAbs > m_Threshold)
		{
			inAbsFilterFast = inAbs - m_Threshold;
			inAbsFilterSlow = m_Threshold;
		}
		else
		{
			inAbsFilterFast = T(0.0);
			inAbsFilterSlow = inAbs;
		}

		// Adjust attacka and release time
		const T diff = std::abs(in - m_OutLast);
		const T timeFactor = T(1.0) + T(0.15) * (diff - T(4.0));
		m_FilterFast.set(timeFactor * m_AttackTime, timeFactor * m_ReleaseTime);

		return m_OutLast = m_FilterFast.process(inAbsFilterFast) + m_FilterSlow.process(inAbsFilterSlow);
	};

protected:
	BranchingEnvelopeFollower<T> m_FilterFast;
	BranchingEnvelopeFollower<T> m_FilterSlow;
	T m_Threshold = T(6.0);

	T m_AttackTime = T(0.0f);
	T m_ReleaseTime = T(0.0);

	T m_OutLast = T(0.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class GainCompensation
{
public:
	GainCompensation() {};

	inline void init(const int sampleRate)
	{
		m_inputEnvelopeFollower.init(sampleRate);
		m_outputEnvelopeFollower.init(sampleRate);

		m_inputEnvelopeFollower.set(T(5.0), T(15.0));
		m_outputEnvelopeFollower.set(T(5.0), T(15.0));
	};
	inline void set(const T dynamics)
	{ 
		m_dynamics = dynamics;
	};
	inline T getGainCompensation(const T in, const T out)
	{
		const T inputLoudness = m_inputEnvelopeFollower.process(in);
		const T outputLoudness = m_outputEnvelopeFollower.process(out);
		T gainCompensation = T(0.0);

		if (outputLoudness > T(0.001))
		{
			gainCompensation = inputLoudness / outputLoudness;

			if (gainCompensation < T(1.0))
			{
				const T delta = T(1.0) - gainCompensation;
				gainCompensation = T(1.0) - (delta * m_dynamics);
			}
			else
			{
				const T delta = gainCompensation - T(1.0);
				gainCompensation = T(1.0) + (delta * m_dynamics);
			}
		}

		return gainComponesation;
	}

private:
	BranchingEnvelopeFollower<T> m_inputEnvelopeFollower;
	BranchingEnvelopeFollower<T> m_outputEnvelopeFollower;

	T m_dynamics = T(0.0);
};