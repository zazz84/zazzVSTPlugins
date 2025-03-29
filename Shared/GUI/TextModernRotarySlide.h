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

#include <vector>

#include <JuceHeader.h>

#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class TextModernRotarySlider : public ModernRotarySlider
{
public:

	TextModernRotarySlider(juce::AudioProcessorValueTreeState& vts, const std::string name, const std::string unit, const std::string label, std::vector<juce::String> names) : ModernRotarySlider(vts, name, unit, label)
	{		
		// Store names
		m_names = names;
		
		// Disable textbox editing
		m_textBox.setEditable(false, false, false);
		
		// Set init text
		const auto value = Math::clampInt((int)m_slider.getValue(), 1, (int)m_names.size() - 1);
		m_textBox.setText(m_names[value], juce::dontSendNotification);

		m_textBox.onTextChange = [this]() { handleTextBoxChange(); };
	};
	~TextModernRotarySlider()
	{
		m_slider.removeListener(this);
	};

private:
	// Update label when slider changes
	void sliderValueChanged(juce::Slider* sliderThatChanged) override
	{
		if (sliderThatChanged == &m_slider)
		{
			const auto value = (int)Math::clamp(m_slider.getValue(), 1, m_names.size()) - 1;
			m_textBox.setText(m_names[value], juce::dontSendNotification);
		}
	}

	std::vector<juce::String> m_names;
};