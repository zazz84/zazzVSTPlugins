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

#include "../../../zazzVSTPlugins/Shared/GUI/SmallSliderComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/CorrelationMeterComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/BalanceMeterComponent.h"

//==============================================================================
class MidSideAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    MidSideAudioProcessorEditor (MidSideAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~MidSideAudioProcessorEditor() override;

	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

private:
    MidSideAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	SmallSliderComponent m_midGainSlider;
	SmallSliderComponent m_sideGainSlider;
	SmallSliderComponent m_midPanSlider;
	SmallSliderComponent m_sidePanSlider;
	SmallSliderComponent m_volumeSlider;

	GroupLabelComponent m_gainLabel;
	GroupLabelComponent m_panLabel;

	CorrelationMeterComponent m_correlationMeter;
	BalanceMeterComponent m_balanceMeter;

	PluginNameComponent m_pluginLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidSideAudioProcessorEditor)
};
