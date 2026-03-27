#pragma once

#include <vector>

#include <JuceHeader.h>

#include "RotarySlider.h"

namespace zazzGUI
{
	class TextRotarySlider : public RotarySlider
	{
	public:

		TextRotarySlider(juce::AudioProcessorValueTreeState& vts, const std::string name, const std::string unit, const std::string label, std::vector<juce::String> names) : RotarySlider(vts, name, unit, label)
		{
			// Store names
			m_names = names;

			// Disable textbox editing
			m_textBox.setEditable(false, false, false);

			// Set init text
			const auto value = (int)m_slider.getValue() - 1;
			m_textBox.setText(m_names[value], juce::dontSendNotification);

			m_textBox.onTextChange = [this]() { handleTextBoxChange(); };
		};
		~TextRotarySlider()
		{
			m_slider.removeListener(this);
		};

	private:
		// Update label when slider changes
		void sliderValueChanged(juce::Slider* sliderThatChanged) override
		{
			if (sliderThatChanged == &m_slider)
			{
				const auto value = (int)m_slider.getValue() - 1;
				m_textBox.setText(m_names[value], juce::dontSendNotification);
			}
		}

		std::vector<juce::String> m_names;
	};
}