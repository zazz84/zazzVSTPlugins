#pragma once

#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Dynamics/TransientShaper.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

class AmbientDelay
{
public:
	AmbientDelay() {};
	inline void init(int size, int sampleRate)
	{
		m_buffer.init(size);
		m_lowPassFilter.init(sampleRate);
		m_highPassFilter.init(sampleRate);
		m_transientShaper.init(sampleRate);
		m_delayTimeShoother.init(sampleRate);
		m_delayTimeShoother.set(2.0f);
		
	};
	inline void set(float size, float lowPassFrequency, float highPassFrequency, float attackGain, float sustainGain, float attacktTime, float clipGain)
	{
		m_size = size;
		constexpr float q = 0.707f;
		m_lowPassFilter.setLowPass(lowPassFrequency, q);
		m_highPassFilter.setHighPass(highPassFrequency, q);
		m_transientShaper.set(attackGain, sustainGain, attacktTime, clipGain);
	};
	inline float process(const float in)
	{		
		const float delayTimeSamples = m_delayTimeShoother.process(m_size);
		float out = m_buffer.readDelayLinearInterpolation(delayTimeSamples);
		m_transientShaper.process(out);
		out = m_lowPassFilter.processDF1(out);
		out = m_highPassFilter.processDF1(out);

		m_buffer.write(in);
		
		return out;
	};

private:
	CircularBuffer m_buffer;
	BiquadFilter m_lowPassFilter;
	BiquadFilter m_highPassFilter;
	TransientShaper m_transientShaper;
	OnePoleLowPassFilter m_delayTimeShoother;

	float m_size = 0.0f;					//Delay in samples
};