#pragma once

#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/Clippers.h"
#include "../../../zazzVSTPlugins/Shared/NonLinearFilters/WaveShapers.h"

class TubeEmulation
{
public:
	TubeEmulation() = default;
	~TubeEmulation() = default;

	inline void init(int sampleRate)
	{
		m_clipper.init(sampleRate);
	};
	inline void set(const float drivedB)
	{ 
		m_drive = juce::Decibels::decibelsToGain(drivedB);
	};
	inline float process(float in)
	{
		float out = m_clipper.process(m_drive * in);
		out = 0.675f * Waveshapers::ARRY(out);
		return out;
	};

private:
	SoftClipper m_clipper;
	float m_drive = 1.0f;
};