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

class ButtonComponent : public juce::Component
{
public:
	ButtonComponent(juce::AudioProcessorValueTreeState& vts, juce::String name, juce::String paramName) : m_button{ name }, m_name(name)
	{
		
		m_button.setClickingTogglesState(true);
		m_button.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(90, 90, 100));
		m_button.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(55, 140, 255));

		m_buttonAAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(vts, paramName, m_button));
		addAndMakeVisible(m_button);
	}
	~ButtonComponent() = default;

	inline void paint(juce::Graphics& g) override
	{
		// Size: 3 x 1
		
		g.fillAll(juce::Colour::fromRGB(90, 90, 100));

		const auto width = getWidth();
		const auto height = getHeight();

		juce::Rectangle<int> bounds;
		bounds.setPosition(width / 4, height / 4);
		bounds.setSize(width / 2, height / 2);

		m_button.setBounds(bounds);
	}

private:
	juce::TextButton m_button;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_buttonAAttachment;
	juce::String m_name;

};