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
#include "../../../zazzVSTPlugins/Shared/GUI/ModernTextButton.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernTextButton.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"

//==============================================================================
class ResynthesizerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ResynthesizerAudioProcessorEditor (ResynthesizerAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~ResynthesizerAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 * 8 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 1 + 4 + 4 + 1 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    ResynthesizerAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernTextButton m_learnButton;

	GroupLabelComponent m_frequencyGroupLabel;
	GroupLabelComponent m_volumeGroupLabel;

	ModernRotarySlider m_frequency1Slider;
	ModernRotarySlider m_frequency2Slider;
	ModernRotarySlider m_frequency3Slider;
	ModernRotarySlider m_frequency4Slider;
	ModernRotarySlider m_frequency5Slider;
	ModernRotarySlider m_frequency6Slider;
	ModernRotarySlider m_frequency7Slider;
	ModernRotarySlider m_frequency8Slider;

	ModernRotarySlider m_volume1Slider;
	ModernRotarySlider m_volume2Slider;
	ModernRotarySlider m_volume3Slider;
	ModernRotarySlider m_volume4Slider;
	ModernRotarySlider m_volume5Slider;
	ModernRotarySlider m_volume6Slider;
	ModernRotarySlider m_volume7Slider;
	ModernRotarySlider m_volume8Slider;

	TextModernRotarySlider m_styleSlider;
	ModernRotarySlider m_shapeSlider;
	ModernRotarySlider m_factorSlider;
	ModernRotarySlider m_minimumStepSlider;
	ModernRotarySlider m_volumeSlider;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResynthesizerAudioProcessorEditor)
};
