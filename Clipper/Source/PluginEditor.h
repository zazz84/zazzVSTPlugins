#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/TextModernRotarySlide.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainReductionMeterComponent.h"

//==============================================================================
class ClipperAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    ClipperAudioProcessorEditor (ClipperAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~ClipperAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 + 3 + 1 + 2 + 1;
	static const int CANVAS_HEIGHT = 2 + 4 + 4 + 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    ClipperAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	TextModernRotarySlider m_typeSlider;
	ModernRotarySlider m_thresholdSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	GainReductionMeterComponent m_gainReductionMeter;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipperAudioProcessorEditor)
};
