#pragma once

#include <math.h>

//==============================================================================
class Helpers
{
	//==============================================================================
	inline static float Remap(float value, float inMin, float inMax, float outMin, float outMax) const
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
	}

	//==============================================================================
	inline static float Limit(float in, float min, float max)
	{
		return std::min(max, std::faxf(in, min));
	}
}