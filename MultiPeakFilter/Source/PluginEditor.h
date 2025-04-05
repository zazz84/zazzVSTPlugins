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
class MultiPeakFilterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MultiPeakFilterAudioProcessorEditor (MultiPeakFilterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~MultiPeakFilterAudioProcessorEditor() override;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    MultiPeakFilterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	CustomLookAndFeel customLook;

	PluginNameComponent m_pluginNameComponent;

	ModernRotarySlider m_frequencySlider;
	ModernRotarySlider m_qSlider;
	ModernRotarySlider m_gainSlider;
	ModernRotarySlider m_stepSlider;
	ModernRotarySlider m_coutSlider;
	ModernRotarySlider m_slopeSlider;
	ModernRotarySlider m_volumeSlider;

	juce::TextButton typeAButton{ "A" };
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAAttachment;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiPeakFilterAudioProcessorEditor)
};
