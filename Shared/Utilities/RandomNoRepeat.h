#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

class RandomNoRepeat {
public:
	RandomNoRepeat(int min, int max, int diffCount)
		: min(min), max(max), diffCount(diffCount)
	{
		std::srand(static_cast<unsigned>(std::time(nullptr)));
	}

	int get() {
		int val;
		do {
			val = min + std::rand() % (max - min + 1);
		} while (isRecent(val));

		// store and trim history
		recent.push_back(val);
		if (recent.size() > static_cast<size_t>(diffCount))
			recent.erase(recent.begin());

		return val;
	}

private:
	int min, max, diffCount;
	std::vector<int> recent;

	bool isRecent(int v) const {
		return std::find(recent.begin(), recent.end(), v) != recent.end();
	}
};