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

#include <JuceHeader.h>

class PluginNameComponent : public juce::Component
{
public:
	PluginNameComponent(juce::String name) : m_name(name)
	{
	}
	~PluginNameComponent() = default;

	inline void paint(juce::Graphics& g) override
	{
		//g.fillAll(juce::Colour::fromRGB(90, 90, 100));

		const auto width = getWidth();
		const auto height = getHeight();

		juce::Rectangle<int> bounds;
		bounds.setPosition(0, 0);
		bounds.setSize(width, height);

		// Group label name
		g.setColour(highlightColor);
		g.setFont(0.5f * static_cast<float>(height));
		g.setOpacity(0.5f);
		g.drawText(m_name, bounds, juce::Justification::centred, false);
	}

private:
	juce::String m_name;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};
