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

class ModernTextButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
	juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
	{
		return juce::Font(10.0f); // Custom font size
	}
};

class ModernTextButton : public juce::Component
{
public:

	ModernTextButton(juce::AudioProcessorValueTreeState& vts, const std::string name) : valueTreeState(vts), m_textButton(name)
	{
		m_textButton.setLookAndFeel(&m_modernTextButtonLookAndFeel);
		addAndMakeVisible(m_textButton);
		m_textButton.setColour(juce::TextButton::buttonColourId, lightColor);
		m_textButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
		m_textButton.setClickingTogglesState(true);

		m_textButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, name, m_textButton));
	}

	inline void paint(juce::Graphics& g) override
	{
		const auto width = getWidth();
		const auto height = getHeight();

		m_textButton.setSize(width, height);
		m_textButton.setTopLeftPosition(0, 0);
	}

	inline void resized() override
	{
		const auto width = getWidth();
		const auto height = getHeight();

		m_textButton.setSize(width, height);
		m_textButton.setTopLeftPosition(0, 0);
	}

	juce::AudioProcessorValueTreeState& valueTreeState;
	ModernTextButtonLookAndFeel m_modernTextButtonLookAndFeel;

	juce::TextButton m_textButton;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_textButtonAttachment;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
};