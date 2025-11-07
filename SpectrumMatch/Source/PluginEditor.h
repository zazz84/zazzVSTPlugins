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
#include "../../../zazzVSTPlugins/Shared/GUI/TextModernRotarySlide.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainMeterComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernTextButton.h"

//==============================================================================
class SpectrumMatchAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    SpectrumMatchAudioProcessorEditor (SpectrumMatchAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~SpectrumMatchAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 6 * 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 8 + 4 + 1 + 1 + 4 + 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    SpectrumMatchAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernRotarySlider m_attackSlider;
	ModernRotarySlider m_releaseSlider;
	ModernRotarySlider m_gain1Slider;
	ModernRotarySlider m_gain2Slider;
	ModernRotarySlider m_gain3Slider;
	ModernRotarySlider m_gain4Slider;
	ModernRotarySlider m_gain5Slider;
	ModernRotarySlider m_gain6Slider;
	ModernRotarySlider m_volumeSlider;
	TextModernRotarySlider m_typeSlider;
	ModernRotarySlider m_mixSlider;

	ModernTextButton m_mute1Button;
	ModernTextButton m_mute2Button;
	ModernTextButton m_mute3Button;
	ModernTextButton m_mute4Button;
	ModernTextButton m_mute5Button;
	ModernTextButton m_mute6Button;

	GainMeterComponent m_gain1Meter;
	GainMeterComponent m_gain2Meter;
	GainMeterComponent m_gain3Meter;
	GainMeterComponent m_gain4Meter;
	GainMeterComponent m_gain5Meter;
	GainMeterComponent m_gain6Meter;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumMatchAudioProcessorEditor)
};
