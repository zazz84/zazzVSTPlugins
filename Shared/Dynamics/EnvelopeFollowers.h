/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

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
		m_sampleRate = sampleRate;
	}
	inline void set(const T attackTimeMs, const T releaseTimeMs)
	{
		m_attackCoef = exp(T(-1000.0) / (attackTimeMs * static_cast<T>(m_sampleRate)));
		m_releaseCoef = exp(T(-1000.0) / (releaseTimeMs * static_cast<T>(m_sampleRate)));
	};
	inline void release()
	{
		m_attackCoef = T(0.0);
		m_releaseCoef = T(0.0);
		m_outLast = T(0.0);
		m_sampleRate = 48000;
	}

protected:
	T m_attackCoef = T(0.0);
	T m_releaseCoef = T(0.0);
	T m_outLast = T(0.0);
	int m_sampleRate = 48000;
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
		const auto coef = (inAbs > m_outLast) ? m_attackCoef : m_releaseCoef;

		return m_outLast = inAbs + coef * (m_outLast - inAbs);
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
		m_OutReleaseLast = std::max(inAbs, inAbs + m_releaseCoef * (m_OutReleaseLast - inAbs));
		
		return m_outLast = m_OutReleaseLast + m_attackCoef * (m_outLast - m_OutReleaseLast);
	};

private:
	T m_OutReleaseLast = T(0.0);
};

//==============================================================================
template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
class HoldEnvelopeFollower
{
public:
	HoldEnvelopeFollower() = default;
	~HoldEnvelopeFollower() = default;

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
	}
	inline void set(const T attackTimeMS, const T releaseTimeMS, const T holdTimeMS)
	{
		m_attackCoef = exp(T(-1000.0) / (attackTimeMS * static_cast<T>(m_sampleRate)));
		m_releaseCoef = exp(T(-1000.0) / (releaseTimeMS * static_cast<T>(m_sampleRate)));

		m_holdTimeSamples = static_cast<int>(T(0.001) * holdTimeMS * static_cast<T>(m_sampleRate));
	};
	inline void setHoldTime(const T holdTimeMS)
	{
		m_holdTimeSamples = static_cast<int>(T(0.001) * holdTimeMS * static_cast<T>(m_sampleRate));
	}
	inline void setHoldTimeSamples(const int holdTimeSamples)
	{
		m_holdTimeSamples = holdTimeSamples;
	}

	T process(T in)
	{
		const T inRectified = std::abs(in);

		// Branching logic
		if (inRectified > m_outLast)
		{
			m_outLast = m_attackCoef * (m_outLast - inRectified) + inRectified;
			m_holdCounter = m_holdTimeSamples; // Reset hold counter
		}
		else
		{
			if (m_holdCounter > 0)
			{
				m_holdCounter--; // Hold the envelope
			}
			else
			{
				m_outLast = m_releaseCoef * (m_outLast - inRectified) + inRectified;
			}
		}

		return m_outLast;

	};

protected:
	T m_attackCoef = T(0.0);
	T m_releaseCoef = T(0.0);
	T m_outLast = T(0.0);
	int m_sampleRate = 48000;
	int m_holdTimeSamples = 0;
	int m_holdCounter = 0;
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
		m_sampleRate = sampleRate;
	}
	inline void set(T attackTimeMs, T releaseTimeMs, T range = T(1.0))
	{
		m_attackCoef = T(1000.0) / (attackTimeMs * static_cast<T>(m_sampleRate) * m_Range);
		m_releaseCoef = T(1000.0) / (releaseTimeMs * static_cast<T>(m_sampleRate) * m_Range);

		m_Range = range;
	}
	inline T process(const T in)
	{
		const T inAbs = std::abs(in);

		if (inAbs > m_outLast)
		{
			const T step = std::min(inAbs - m_outLast, m_attackCoef);
			return m_outLast += step;
		}
		else
		{
			const T step = std::min(m_outLast - inAbs, m_releaseCoef);
			return m_outLast -= step;
		}
	}

protected:
	T m_attackCoef = T(0.0);
	T m_releaseCoef = T(0.0);
	T m_outLast = T(0.0);
	T m_Range = T(1.0);
	int m_sampleRate = 48000;
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
		m_sampleRate = sampleRate;
	}
	inline void set(T attackTimeMs, T releaseTimeMs)
	{
		m_attackTime = attackTimeMs;
		m_releaseTime = releaseTimeMs;
	};
	inline T process(T in)
	{
		updateCoef();

		const T inAbs = std::abs(in);
		m_out1Last = std::max(inAbs, m_releaseCoef * m_out1Last + (static_cast<T>(1.0) - m_releaseCoef) * inAbs);
		
		return m_outLast = m_attackCoef * (m_outLast - m_out1Last) + m_out1Last;
	};

protected:
	void updateCoef()
	{
		// Attack and release time gets shorter with increasing gain reduction
		const T attackTimeMs = m_attackTime * (T(3.0) / (m_outLast + T(3.0)));
		const T releaseTimeMs = m_releaseTime * (T(2.5) / (m_outLast + T(2.5)));

		m_attackCoef = exp(T(-1000.0) / (attackTimeMs * static_cast<T>(m_sampleRate)));
		m_releaseCoef = exp(T(-1000.0) / (releaseTimeMs * static_cast<T>(m_sampleRate)));
	};

	T m_attackCoef = T(0.0);
	T m_releaseCoef = T(0.0);
	T m_attackTime = T(0.0);
	T m_releaseTime = T(0.0);
	T m_outLast = T(0.0);
	T m_out1Last = T(0.0);
	int  m_sampleRate = 48000;
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
		m_filterFast.init(sampleRate);
		m_filterSlow.init(sampleRate);
	};
	inline void set(const T attackTimeMs, const T releaseTimeMs)
	{
		m_attackTime = attackTimeMs;
		m_releaseTime = releaseTimeMs;
		
		m_filterFast.set(attackTimeMs, releaseTimeMs);
		m_filterSlow.set(static_cast<T>(150.0), static_cast<T>(600.0));
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
		const T diff = std::abs(in - m_outLast);
		const T timeFactor = T(1.0) + T(0.15) * (diff - T(4.0));
		m_filterFast.set(timeFactor * m_attackTime, timeFactor * m_releaseTime);

		return m_outLast = m_filterFast.process(inAbsFilterFast) + m_filterSlow.process(inAbsFilterSlow);
	};

protected:
	BranchingEnvelopeFollower<T> m_filterFast;
	BranchingEnvelopeFollower<T> m_filterSlow;
	T m_Threshold = T(6.0);
	T m_attackTime = T(0.0f);
	T m_releaseTime = T(0.0);
	T m_outLast = T(0.0);
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