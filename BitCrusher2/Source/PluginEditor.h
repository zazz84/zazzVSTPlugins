/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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
class BitCrusher2AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BitCrusher2AudioProcessorEditor (BitCrusher2AudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~BitCrusher2AudioProcessorEditor() override;

	// GUI setup
	static const unsigned int SLIDER_COUNT = 5;
	static const unsigned int CANVAS_WIDTH = 1 + SLIDER_COUNT * 3 + 1;
	static const unsigned int CANVAS_HEIGHT = 2 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    BitCrusher2AudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernRotarySlider m_bitDepthSlider;
	ModernRotarySlider m_driveSlider;
	ModernRotarySlider m_downsampleSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BitCrusher2AudioProcessorEditor)
};
