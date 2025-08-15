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
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"

//==============================================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
	juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
	{
		return juce::Font(10.0f); // Custom font size
	}
};

//==============================================================================
class RumbleAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RumbleAudioProcessorEditor (RumbleAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~RumbleAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 + 3 + 3 + 3 + 3 + 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 1 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    RumbleAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	CustomLookAndFeel customLook;

	PluginNameComponent m_pluginLabel;

	GroupLabelComponent m_triggerGroupLabel;
	GroupLabelComponent m_toneGroupLabel;

	ModernRotarySlider m_thresholdSlider;
	ModernRotarySlider m_lenghtSlider;
	ModernRotarySlider m_pitchSlider;
	ModernRotarySlider m_delaySlider;
	ModernRotarySlider m_amountSlider;
	ModernRotarySlider m_volumeSlider;

	juce::TextButton m_soloButton{ "S" };
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_soloButtonAttachment;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RumbleAudioProcessorEditor)
};
