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
#include "../../../zazzVSTPlugins/Shared/GUI/CorrelationMeterComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/BalanceMeterComponent.h"

//==============================================================================
class DynamicMidSideAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    DynamicMidSideAudioProcessorEditor (DynamicMidSideAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~DynamicMidSideAudioProcessorEditor() override;

	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

private:
    DynamicMidSideAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	ModernRotarySlider m_speedSlider;
	ModernRotarySlider m_widthSlider;
	ModernRotarySlider m_midGainSlider;
	ModernRotarySlider m_sideGainSlider;
	ModernRotarySlider m_midPanSlider;
	ModernRotarySlider m_sidePanSlider;
	ModernRotarySlider m_volumeSlider;

	GroupLabelComponent m_dynamicLabel;
	GroupLabelComponent m_gainLabel;
	GroupLabelComponent m_panLabel;

	CorrelationMeterComponent m_correlationMeter;
	BalanceMeterComponent m_balanceMeter;

	PluginNameComponent m_pluginLabel;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicMidSideAudioProcessorEditor)
};
