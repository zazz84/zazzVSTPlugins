#pragma once

#include <JuceHeader.h>

#include "../classes/Colors.h"

namespace zazzGUI
{
	class RotarySliderTextBox : public juce::Label
	{
	public:
		RotarySliderTextBox(const juce::String& componentName, const juce::String& initialText) : juce::Label(componentName, initialText)
		{
			setEditable(true, true, false);																// Allow editing on single or double-click
			setJustificationType(juce::Justification::centred);
			setColour(juce::Label::backgroundColourId, zazzGUI::Colors::darkColor);
			setColour(juce::Label::textColourId, juce::Colours::white);
			setColour(juce::Label::outlineColourId, zazzGUI::Colors::darkColor);
			setColour(juce::Label::outlineWhenEditingColourId, juce::Colour::fromRGB(90, 90, 100));
		}

		void textWasEdited() override
		{
			// Handle the text change when the user finishes editing
			// You can add additional logic here, such as notifying a listener
		}

		void paint(juce::Graphics& g) override
		{
			//g.fillAll(juce::Colour::fromRGB(90, 90, 100));
			
			g.setFont(0.8f * static_cast<float>(getHeight()));
			g.setColour(juce::Colours::white);
			g.drawText(getText(), getLocalBounds(), getJustificationType(), true);
		}

		// Override createEditorComponent to customize the TextEditor
		juce::TextEditor* createEditorComponent() override
		{
			auto* editor = new juce::TextEditor();

			// Set the font and justification for the TextEditor
			editor->setFont(getFont());  // Set font size to match the label
			editor->setJustification(getJustificationType());  // Apply justification during editing
			
			// Set the background color of the TextEditor
			editor->setColour(juce::TextEditor::backgroundColourId, zazzGUI::Colors::darkColor);

			// Set the outline color of the TextEditor
			editor->setColour(juce::TextEditor::outlineColourId, zazzGUI::Colors::darkColor);
			editor->setColour(juce::TextEditor::focusedOutlineColourId, zazzGUI::Colors::darkColor);

			return editor;
		}
	};
}