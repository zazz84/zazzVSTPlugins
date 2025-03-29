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
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/LimiterTypeModernRotarySlide.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainReductionMeterComponent.h"

//==============================================================================
class LimiterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    LimiterAudioProcessorEditor (LimiterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~LimiterAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 + 3 + 3 + 2 + 1;
	static const int CANVAS_HEIGHT = 2 + 4 + 4 + 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    LimiterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	ModernRotarySlider m_gainSlider;
	ModernRotarySlider m_thresholdSlider;
	ModernRotarySlider m_volumeSlider;
	LimiterTypeModernRotarySlider m_typeSlider;
	ModernRotarySlider m_releaseSlider;

	PluginNameComponent m_pluginLabel;

	GainReductionMeterComponent m_gainReductionMeter;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessorEditor)
};
