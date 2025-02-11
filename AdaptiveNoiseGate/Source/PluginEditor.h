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


 //==============================================================================
class SensitivityMeterComponent : public juce::Component
{
public:
	SensitivityMeterComponent() = default;
	~SensitivityMeterComponent() = default;

	inline void paint(juce::Graphics& g) override
	{
		juce::Rectangle<float> rectangle;
		rectangle.setSize(100, 100);
		
		//rectangle.setSize(getWidth() * m_difference / 2.0f, getHeight());
		//g.setColour(juce::Colours::white);
		//g.fillRect(rectangle);

		g.drawText(juce::String(m_difference), rectangle, juce::Justification::centred);
	};
	inline void set(float difference)
	{
		m_difference = difference;
	};

private:
	float m_difference = 0.0f;
};

//==============================================================================
class AdaptiveNoiseGateAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor, public juce::Timer
{
public:
    AdaptiveNoiseGateAudioProcessorEditor (AdaptiveNoiseGateAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~AdaptiveNoiseGateAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 5;
	static const int SLIDERS[];
	static const float COLUMN_OFFSET[];
	static const int N_ROWS = 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    AdaptiveNoiseGateAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_pluginName;
	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	SensitivityMeterComponent m_sensitivityMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdaptiveNoiseGateAudioProcessorEditor)
};


