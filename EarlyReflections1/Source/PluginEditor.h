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
class EarlyRefections1AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EarlyRefections1AudioProcessorEditor (EarlyRefections1AudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~EarlyRefections1AudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 8 * 3 + 1 + 1;
	static const int CANVAS_HEIGHT = 2 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    EarlyRefections1AudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernRotarySlider m_predelaySlider;
	ModernRotarySlider m_lenghtSlider;
	ModernRotarySlider m_decaySlider;
	ModernRotarySlider m_dampingSlider;
	ModernRotarySlider m_diffusionSlider;
	ModernRotarySlider m_widthSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EarlyRefections1AudioProcessorEditor)
};
