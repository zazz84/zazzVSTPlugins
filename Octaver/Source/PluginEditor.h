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
class OctaverAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    OctaverAudioProcessorEditor (OctaverAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~OctaverAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 + 3 + 3 + 3 +3 + 3 + 3 + 3 + 3 + 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    OctaverAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	GroupLabelComponent m_noiseGateGroupLabel;
	GroupLabelComponent m_envelopeGroupLabel;
	GroupLabelComponent m_frequencyGroupLabel;
	GroupLabelComponent m_generatorGroupLabel;

	ModernRotarySlider m_noiseGateThresholdSlider;
	ModernRotarySlider m_noiseGateSpeedSlider;
	ModernRotarySlider m_envelopeSpeedSlider;
	ModernRotarySlider m_generatorFrequencyMinSlider;
	ModernRotarySlider m_generatorFrequencyMaxSlider;
	ModernRotarySlider m_generatorSpeedSlider;
	ModernRotarySlider m_generatorPitchSlider;
	ModernRotarySlider m_generatorShapeSlider;
	ModernRotarySlider m_generatorVolumeSlider;
	ModernRotarySlider m_directVolumeSlider;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OctaverAudioProcessorEditor)
};
