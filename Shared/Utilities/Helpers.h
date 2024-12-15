#pragma once

#include <math.h>

#define SPEED_OF_SOUND 343.0f

//==============================================================================
class Helpers
{
public:
	//==============================================================================
	inline static float Remap(float value, float inMin, float inMax, float outMin, float outMax)
	{
		if (value <= inMin)
		{
			return outMin;
		}
		else if (value >= inMax)
		{
			return outMax;
		}
		else
		{
			return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
		}
	};

	//==============================================================================
	inline static float Limit(float in, float min, float max)
	{
		if (in > max)
		{
			return max;
		}
		else if (in < min)
		{
			return min;
		}
		else
		{
			return in;
		}
	};

	//==============================================================================
	inline static float GetDistanceAttenuation(const float distance, const float attenuationFactor)
	{
		return attenuationFactor / (distance + attenuationFactor);
	}

	//==============================================================================
	inline static void PanMonoToStereo(const float in, const float pan, float& left, float& right)
	{
		const float  panFactor = 0.5f * (pan + 1.0f);
		
		left = in * (1.0 - panFactor);
		right = in * panFactor;
	}
};