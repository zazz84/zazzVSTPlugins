#pragma once

#include <JuceHeader.h>

#include "Colors.h"

namespace zazzGUI
{
	class TextButtonLookAndFeel : public juce::LookAndFeel_V4
	{
	public:
		juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
		{
			return juce::Font(10.0f); // Custom font size
		}
	};

	class TextButton : public juce::Component
	{
	public:

		TextButton(juce::AudioProcessorValueTreeState& vts, const std::string name) : valueTreeState(vts), m_textButton(name)
		{
			m_textButton.setLookAndFeel(&m_textButtonLookAndFeel);
			addAndMakeVisible(m_textButton);
			m_textButton.setColour(juce::TextButton::buttonColourId, zazzGUI::Colors::darkColor);
			m_textButton.setColour(juce::TextButton::buttonOnColourId, zazzGUI::Colors::lightColor);
			m_textButton.setClickingTogglesState(true);

			m_textButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, name, m_textButton));
		}

		TextButton(juce::AudioProcessorValueTreeState& vts, const std::string label, std::string name) : valueTreeState(vts), m_textButton(label)
		{
			m_textButton.setLookAndFeel(&m_textButtonLookAndFeel);
			addAndMakeVisible(m_textButton);
			m_textButton.setColour(juce::TextButton::buttonColourId, zazzGUI::Colors::darkColor);
			m_textButton.setColour(juce::TextButton::buttonOnColourId, zazzGUI::Colors::lightColor);
			m_textButton.setClickingTogglesState(true);

			m_textButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, name, m_textButton));
		}

		inline void paint(juce::Graphics& g) override
		{
			const auto width = getWidth();
			const auto height = getHeight();

			const auto border = height * m_border / 100;

			m_textButton.setSize(width - border - border, height - border - border);
			m_textButton.setTopLeftPosition(border, border);
		}

		inline void resized() override
		{
			const auto width = getWidth();
			const auto height = getHeight();

			const auto border = height * m_border / 100;

			m_textButton.setSize(width - border - border, height - border - border);
			m_textButton.setTopLeftPosition(border, border);
		}

		void setRadioGroupId(int newGroupId)
		{
			m_textButton.setRadioGroupId(newGroupId);
		}

		void setBorder(int border)
		{
			m_border = border;
		}

		void setClickingTogglesState(bool state)
		{
			m_textButton.setClickingTogglesState(state);
		}


	private:
		juce::AudioProcessorValueTreeState& valueTreeState;
		TextButtonLookAndFeel m_textButtonLookAndFeel;

		juce::TextButton m_textButton;
		std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_textButtonAttachment;

		int m_border = 0; // [0, 100] percentage
	};
}