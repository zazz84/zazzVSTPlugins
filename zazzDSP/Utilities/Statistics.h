#pragma once

#include <vector>
#include <algorithm>

namespace zazzDSP
{
	class Statistics
	{
	public:
		inline static int getMedian(std::vector<int> input)
		{
			if (input.empty())
			{
				return 0;
			}

			std::sort(input.begin(), input.end());
			size_t n = input.size();

			if (n % 2 == 1) // odd number of elements
			{
				return input[n / 2];
			}
			else // even number of elements
			{
				return (input[n / 2 - 1] + input[n / 2]) / 2;
			}
		}
	};
}
