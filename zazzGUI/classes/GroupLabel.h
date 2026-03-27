#pragma once

#include <JuceHeader.h>

#include "Colors.h"

namespace zazzGUI
{
	class GroupLabel : public juce::Component
	{

	public:
		GroupLabel(const juce::String& name) : m_name(name)
		{
		}
		~GroupLabel() = default;

		static const int MICRO_LOD_HEIGHT_LIMIT = 17;

		inline void paint(juce::Graphics& g) override
		{
			// Size: n x 1

			g.fillAll(zazzGUI::Colors::darkColor);

			const auto width = getWidth();
			const auto height = getHeight();

			if (height > MICRO_LOD_HEIGHT_LIMIT)
			{
				juce::Rectangle<int> bounds;
				bounds.setPosition(0, 0);
				bounds.setSize(width, height);

				// Group label name
				g.setColour(zazzGUI::Colors::highlightColor);
				g.setFont(FONT_SIZE_RATIO * static_cast<float>(height));
				g.setOpacity(TEXT_OPACITY);
				g.drawText(m_name, bounds, juce::Justification::centred, false);


				// Draw lines
				int textWidth = g.getCurrentFont().getStringWidth(m_name) + height / 2;

				int yPosition = height / 2;

				int xLeftStart = height / 2;
				int xLeftStop = (width - textWidth) / 2;
				int xRightStart = (width + textWidth) / 2;
				int xRightStop = width - (height / 2);

				g.setOpacity(1.0f);
				g.drawLine(static_cast<float>(xLeftStart), static_cast<float>(yPosition), static_cast<float>(xLeftStop), static_cast<float>(yPosition), LINE_WIDTH);			// Line width = 1.0f
				g.drawLine(static_cast<float>(xRightStart), static_cast<float>(yPosition), static_cast<float>(xRightStop), static_cast<float>(yPosition), LINE_WIDTH);		// Line width = 1.0f
			}
			else
			{
				int yPosition = height / 2;

				int xStart = height / 2;
				int xStop = width - (height / 2);

				g.setColour(juce::Colours::white);
				g.drawLine(static_cast<float>(xStart), static_cast<float>(yPosition), static_cast<float>(xStop), static_cast<float>(yPosition), LINE_WIDTH);			// Line width = 1.0f
			}
		}

		inline void resized() override
		{
		}

	private:
		juce::String m_name;

		static constexpr float FONT_SIZE_RATIO = 0.4f;
		static constexpr float TEXT_OPACITY = 0.5f;
		static constexpr float LINE_WIDTH = 0.5f;
	};
}