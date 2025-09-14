#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/StereoWidthMeterComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"

//==============================================================================
class MonoToStereoAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    MonoToStereoAudioProcessorEditor (MonoToStereoAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~MonoToStereoAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 5 * 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 4 + 2 + 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint(juce::Graphics&) override;
	void resized() override;
	
protected:
    MonoToStereoAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	GroupLabelComponent m_colorGroupLabel;
	GroupLabelComponent m_dynamicGroupLabel;
	GroupLabelComponent m_modulationGroupLabel;

	StereoWidthMeterComponent m_stereoWidthMeter;

	ModernRotarySlider m_delaySlider;
	ModernRotarySlider m_widthSlider;
	ModernRotarySlider m_colorSlider;
	ModernRotarySlider m_HPSlider;
	ModernRotarySlider m_dynamicSlider;
	ModernRotarySlider m_dynamicSpeedSlider;
	ModernRotarySlider m_modulationDepthSlider;
	ModernRotarySlider m_modulationSpeedSlider;
	ModernRotarySlider m_volumeSlider;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoToStereoAudioProcessorEditor)
};
