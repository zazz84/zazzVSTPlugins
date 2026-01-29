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
#include "../../../zazzVSTPlugins/Shared/GUI/ModernTextButton.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/SpectrumCurveComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/SpectrumDifferenceComponent.h"

//==============================================================================
class SpectrumMatchFFTAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    SpectrumMatchFFTAudioProcessorEditor (SpectrumMatchFFTAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~SpectrumMatchFFTAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 30 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 10 + 1 + 10 + 1 + 4;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    SpectrumMatchFFTAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	GroupLabelComponent m_spectrumGroupLabel;
	GroupLabelComponent m_spectrumDiffGroupLabel;

	SpectrumCurveComponent m_spectrumCurveComponent;
	SpectrumDifferenceComponent m_spectrumDifferenceComponent;

	ModernRotarySlider m_lowPassFilterSlider;
	ModernRotarySlider m_highPassFilterSlider;
	ModernRotarySlider m_frequencyShiftSlider;
	ModernRotarySlider m_resolutionSlider;
	ModernRotarySlider m_ammountSlider;
	ModernRotarySlider m_volumeSlider;

	ModernTextButton m_sourceSpectrumButton;
	ModernTextButton m_targetSpectrumButton;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumMatchFFTAudioProcessorEditor)
};
