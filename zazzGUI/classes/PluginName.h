#pragma once

#include <JuceHeader.h>

#include "Colors.h"

namespace zazzGUI
{
	class PluginName : public juce::Component
	{
	public:
		PluginName(const juce::String& name) : m_name(name)
		{
		}
		~PluginName() = default;

		inline void paint(juce::Graphics& g) override
		{
			const auto width = getWidth();
			const auto height = getHeight();

			juce::Rectangle<int> bounds;
			bounds.setPosition(0, 0);
			bounds.setSize(width, height);

			// Group label name
			static constexpr float FONT_SIZE_RATIO = 0.5f;
			static constexpr float TEXT_OPACITY = 0.5f;

			g.setColour(zazzGUI::Colors::highlightColor);
			g.setFont(FONT_SIZE_RATIO * static_cast<float>(height));
			g.setOpacity(TEXT_OPACITY);
			g.drawText(m_name, bounds, juce::Justification::centred, false);
		}

	private:
		juce::String m_name;
	};
}
