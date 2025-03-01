#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainMeterComponent.h"

//==============================================================================
class TransientShaperAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    TransientShaperAudioProcessorEditor (TransientShaperAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~TransientShaperAudioProcessorEditor() override;
	
	static const int CANVAS_WIDTH = 1 + 3 + 2 + 3 + 1 + 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 4 + 1;

	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    TransientShaperAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	ModernRotarySlider m_attackLenghtSlider;
	ModernRotarySlider m_attackSlider;
	ModernRotarySlider m_sustainLenghtSlider;
	ModernRotarySlider m_sustainSlider;
	ModernRotarySlider m_volumeSlider;

	GroupLabelComponent m_attackLabel;
	GroupLabelComponent m_sustainLabel;

	PluginNameComponent m_pluginLabel;

	GainMeterComponent m_gainMeter;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransientShaperAudioProcessorEditor)
};
