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
#include "../../../zazzVSTPlugins/Shared/GUI/FrequencyMeterComponent.h"
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
class PitchTrackingEQAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    PitchTrackingEQAudioProcessorEditor (PitchTrackingEQAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~PitchTrackingEQAudioProcessorEditor() override;

	static const int TYPE_BUTTON_GROUP = 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    PitchTrackingEQAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginNameComponent;
	FrequencyMeterComponent m_frequencyMeterComponent;
	GroupLabelComponent m_filterGroupLabel;

	ModernRotarySlider m_detectorFrequencyMinSlider;
	ModernRotarySlider m_detectorFrequencyMaxSlider;
	ModernRotarySlider m_smootherSpeedSlider;
	ModernRotarySlider m_filterFrequencyMultiplierSlider;
	ModernRotarySlider m_filterQSlider;
	ModernRotarySlider m_filterGainSlider;
	ModernRotarySlider m_volumeSlider;

	CustomLookAndFeel customLook;

	juce::TextButton typeAButton{ "LP" };
	juce::TextButton typeBButton{ "HP" };
	juce::TextButton typeCButton{ "P" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonBAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonCAttachment;

	std::atomic<float>* frequencyMinParameter = nullptr;
	std::atomic<float>* frequencyMaxParameter = nullptr;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchTrackingEQAudioProcessorEditor)
};
