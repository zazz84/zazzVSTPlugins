#pragma once

constexpr auto SoftClipperThresholdPositive = 0.7f;
constexpr auto SoftClipperThresholdNegative = -0.7f;
constexpr auto SoftClipperThresholdRatio = 0.2f;

class  WaveShaper
{
public:
	WaveShaper() {};

	inline void setSaturation(float saturation) { m_saturation = saturation; };

	inline float processARRY(float in)
	{ 
		return 1.5f * in * (1.0f - in * in / 3.0f);
	};

private:
	float m_saturation = 1.0f;
};

class SoftClipper
{
public:
	SoftClipper() {};

	inline float process(float in)
	{
		float out = 0.0f;
		
		if (in > SoftClipperThresholdPositive)
		{
			out = SoftClipperThresholdPositive + (in - SoftClipperThresholdPositive) * SoftClipperThresholdRatio;
		}
		else if (in < SoftClipperThresholdNegative)
		{
			out = SoftClipperThresholdNegative + (in - SoftClipperThresholdNegative) * SoftClipperThresholdRatio;
		}
		else
		{
			out = in;
		}

		return fmaxf(-1.0f, fminf(1.0f, out));
	};
};


class TriodeAClass
{
public:
	TriodeAClass() {};

	inline void setDrive(float drive) { m_drive = juce::Decibels::decibelsToGain(drive); };

	inline float process(float in)
	{
		float out = m_softClipper.process(in * m_drive);
		out = 0.707f * m_waveShaper.processARRY(out);
		return out;
	};

private:
	float m_drive;
	WaveShaper m_waveShaper;
	SoftClipper m_softClipper;
};