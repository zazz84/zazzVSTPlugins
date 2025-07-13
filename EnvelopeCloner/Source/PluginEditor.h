#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainMeterComponent.h"

//==============================================================================
class EnvelopeClonerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    EnvelopeClonerAudioProcessorEditor (EnvelopeClonerAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~EnvelopeClonerAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 2 * 3 + 1 + 3 * 2 + 1;
	static const int CANVAS_HEIGHT = 2 + 4 + 4 + 4 + 1;
	
	//==============================================================================
	void timerCallback() override;
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    EnvelopeClonerAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	ModernRotarySlider m_dynamicsSlider;
	ModernRotarySlider m_spectrumSlider;
	ModernRotarySlider m_attackSlider;
	ModernRotarySlider m_releaseSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	GainMeterComponent m_gainMeterLow;
	GainMeterComponent m_gainMeterMid;
	GainMeterComponent m_gainMeterHigh;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeClonerAudioProcessorEditor)
};
