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

class NoteModernRotarySlider : public ModernRotarySlider
{
public:

	NoteModernRotarySlider(juce::AudioProcessorValueTreeState& vts, const std::string name, const std::string unit, const std::string label) : ModernRotarySlider(vts, name, unit, label)
	{
		// Disable textbox editing
		m_textBox.setEditable(false, false, false);

		// Set init text
		setTextBox();

		m_textBox.onTextChange = [this]() { handleTextBoxChange(); };
	};
	~NoteModernRotarySlider()
	{
		m_slider.removeListener(this);
	};

private:
	// Update label when slider changes
	void sliderValueChanged(juce::Slider* sliderThatChanged) override
	{
		if (sliderThatChanged == &m_slider)
		{
			setTextBox();
		}
	}

	void setTextBox()
	{
		const auto midiNote = (int)m_slider.getValue();
		juce::String noteName = juce::MidiMessage::getMidiNoteName(midiNote, true, true, 4);
		const int frequency = static_cast<int>(Math::noteToFrequency(midiNote));
		juce::String frequencyString = frequency < 1000 ? juce::String(frequency) : juce::String(0.001f * (float)frequency, 1) + "k";
		m_textBox.setText(noteName + " (" + frequencyString + "Hz)", juce::dontSendNotification);
	}
};