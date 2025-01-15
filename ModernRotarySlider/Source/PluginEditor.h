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

//==============================================================================
/**
*/
class ModernRotarySliderAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ModernRotarySliderAudioProcessorEditor (ModernRotarySliderAudioProcessor&);
    ~ModernRotarySliderAudioProcessorEditor() override;

    //==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ModernRotarySliderAudioProcessor& audioProcessor;

	ModernRotarySlider m_slider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModernRotarySliderAudioProcessorEditor)
};
