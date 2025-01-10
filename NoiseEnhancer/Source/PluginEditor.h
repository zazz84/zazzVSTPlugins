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
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

#include "../../../zazzVSTPlugins/Shared/GUI/SmallSliderComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ButtonComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PeakLevelMeterComponent.h"

//==============================================================================
class NoiseEnhancerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    NoiseEnhancerAudioProcessorEditor (NoiseEnhancerAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~NoiseEnhancerAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 14;
	static const int SLIDERS[];
	static const float COLUMN_OFFSET[];
	static const int N_ROWS = 3;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    NoiseEnhancerAudioProcessor& audioProcessor;
	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_pluginName;
	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	SmallSliderComponent m_attackSlider;
	SmallSliderComponent m_decaySlider;
	SmallSliderComponent m_sustainkSlider;
	SmallSliderComponent m_sustainLevelSlider;
	SmallSliderComponent m_releaseSlider;

	SmallSliderComponent m_freqA0Slider;
	SmallSliderComponent m_freqA1Slider;
	SmallSliderComponent m_freqDSlider;
	SmallSliderComponent m_freqSSlider;
	SmallSliderComponent m_freqRSlider;

	SmallSliderComponent m_frequencySlider;
	SmallSliderComponent m_thresholdSlider;
	SmallSliderComponent m_amountSlider;
	SmallSliderComponent m_volumeSlider;

	GroupLabelComponent m_amplitudeEnvelopeLabel;
	GroupLabelComponent m_frequencyEnvelopeLabel;
	GroupLabelComponent m_triggerLabel;
	GroupLabelComponent m_outputLabel;

	ButtonComponent m_triggerSoloButton;
	ButtonComponent m_noiseSoloButton;

	PeakLevelMeterComponent m_peakMeter;

	PluginNameComponent m_pluginLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseEnhancerAudioProcessorEditor)
};
