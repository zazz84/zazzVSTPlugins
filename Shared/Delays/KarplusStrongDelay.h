#pragma once

#include <cmath>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

class KarplusStrongDelay
{
public:
	KarplusStrongDelay() {};

	inline void init(const int sampleRate, const float frequencyMin)
	{
		m_sampleRate = sampleRate;

		m_buffer.init((int)((float)sampleRate / frequencyMin));
		m_filter.init(sampleRate);
		m_postFilter.init(sampleRate);
	};
	inline void set(const float frequency, const float lenghtSeconds)
	{
		m_buffer.set((int)((float)m_sampleRate / frequency));
		m_filter.setLowPass(2.0f * frequency, 0.7f);
		m_postFilter.setBandPassPeakGain(frequency, 1.0f);

		// Not sure this works correctly
		m_decayFactor = std::expf(-6.9078f / (frequency * lenghtSeconds));
	};
	inline float process(const float in)
	{
		const float delay = m_buffer.read();
		const float filter = m_filter.processDF1(delay);
		const float mix = (in + filter) * m_decayFactor;
		m_buffer.write(mix);

		return m_postFilter.processDF1(filter);
	};

private:
	CircularBuffer m_buffer;
	BiquadFilter m_filter;
	BiquadFilter m_postFilter;
	float m_decayFactor = 0.85f;
	int m_sampleRate = 48000;
};