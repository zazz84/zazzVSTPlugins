#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GainReductionMeterComponent.h"

//==============================================================================
class VocalCompressorAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
	VocalCompressorAudioProcessorEditor(VocalCompressorAudioProcessor&, juce::AudioProcessorValueTreeState&);
	~VocalCompressorAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 2 + 3 + 3 + 2 + 2;
	static const int CANVAS_HEIGHT = 2 + 4 + 4 + 1;

	//==============================================================================
	void timerCallback() override;
	void paint(juce::Graphics&) override;
	void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

protected:
	VocalCompressorAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	ModernRotarySlider m_gainSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	PluginNameComponent m_pluginLabel;

	GainReductionMeterComponent m_gainReductionMeter;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalCompressorAudioProcessorEditor)
};
