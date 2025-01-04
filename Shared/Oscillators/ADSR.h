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

#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

 //==============================================================================
class Envelope
{
public:
	Envelope() = default;
	~Envelope() = default;

	struct EnvelopeParams
	{
	public:
		EnvelopeParams() = default;
		EnvelopeParams(
			float attackTimeMS,
			float decayTimeMS,
			float sustainTimeMS,
			float releaseTimeMS,
			float beginning,
			float attack,
			float decay,
			float sustain,
			float release)
			: m_attackTimeMS(attackTimeMS),
			m_decayTimeMS(decayTimeMS),
			m_sustainTimeMS(sustainTimeMS),
			m_releaseTimeMS(releaseTimeMS),
			m_beggining(beginning),
			m_attack(attack),
			m_decay(decay),
			m_sustain(sustain),
			release(release)
		{
		}
		~EnvelopeParams() = default;

		float m_attackTimeMS = 0.0f;
		float m_decayTimeMS = 0.0f;
		float m_sustainTimeMS = 0.0f;
		float m_releaseTimeMS = 0.0f;

		float m_beggining = 0.0f;
		float m_attack = 0.0f;
		float m_decay = 0.0f;
		float m_sustain = 0.0f;
		float release = 0.0f;
	};

	inline void init(const int sampleRate)
	{
		m_sampleRate = sampleRate;
		// Set phase to large value, so envelope does not play after init
		m_phase = 100 * sampleRate;
	};
	inline void set(const EnvelopeParams& params)
	{
		const float srMS = 0.001f * static_cast<float>(m_sampleRate);

		m_attackSamples = static_cast<int>(srMS * params.m_attackTimeMS);
		m_decaySamples = m_attackSamples + static_cast<int>(srMS * params.m_decayTimeMS);
		m_sustainSamples = m_decaySamples + static_cast<int>(srMS * params.m_sustainTimeMS);
		m_releaseSamples = m_sustainSamples + static_cast<int>(srMS * params.m_releaseTimeMS);

		m_beggining = params.m_beggining;

		m_attackStep = (params.m_attack - params.m_beggining) / static_cast<float>(m_attackSamples);
		m_decayStep = (params.m_decay - params.m_attack) / static_cast<float>(m_decaySamples);
		m_sustainStep = (params.m_sustain - params.m_decay) / static_cast<float>(m_decaySamples);
		m_releaseStep = (params.m_releaseTimeMS - params.m_sustain) / static_cast<float>(m_releaseSamples);
	};
	inline float process()
	{		
		if (m_phase < m_attackSamples)
		{
			m_out += m_attackStep;
		}
		else if (m_phase < m_decaySamples)
		{
			m_out += m_sustainSamples;
		}
		else if (m_phase < m_sustainSamples)
		{
			m_out += m_sustainStep;
		}
		else if (m_phase < m_releaseSamples)
		{
			m_out += m_releaseStep;
		}

		m_phase++;

		return m_out;
	};
	inline void release()
	{
		m_attackSamples = 0;
		m_decaySamples = 0;
		m_sustainSamples = 0;
		m_releaseSamples = 0;
		m_sampleRate = 48000;
	};
	inline void reset()
	{
		m_out = m_beggining;
		m_phase = 0;
	};
	inline void resetSustain()
	{
		m_phase = m_attackSamples;
	};
	inline bool isFinnished()
	{
		return m_phase >= m_releaseSamples;
	}
	inline bool isAttackFinnished()
	{
		return m_phase >= m_attackSamples;
	}
	inline bool isDecayFinnished()
	{
		return m_phase >= m_decaySamples;
	}
	inline bool isSustainFinnished()
	{
		return m_phase >= m_sustainSamples;
	}

protected:
	float m_out = 0.0f;
	float m_beggining = 0.0f;
	float m_attackStep = 0.0f;
	float m_decayStep = 0.0f;
	float m_sustainStep = 0.0f;
	float m_releaseStep = 0.0f;
	int m_phase = 0;
	int m_attackSamples = 0;
	int m_decaySamples = 0;
	int m_sustainSamples = 0;
	int m_releaseSamples = 0;
	int m_sampleRate = 48000;
};

//==============================================================================
class AmplitudeEnvelope : public Envelope
{
public:
	AmplitudeEnvelope() = default;
	~AmplitudeEnvelope() = default;

	inline void set(const float attackTimeMS, const float decayTimeMS, const float sustainTimeMS, const float releaseTimeMS, const float sustaindB)
	{
		const float sustainGain = Math::dBToGain(sustaindB);
		
		const EnvelopeParams params(attackTimeMS,
									decayTimeMS,
									sustainTimeMS,
									releaseTimeMS,
									0.0f,
									1.0f,
									sustainGain,
									sustainGain,
									0.0f);

		__super::set(params);
	};
};