#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/TextModernRotarySlide.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"

//==============================================================================
class NoiseGeneratorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NoiseGeneratorAudioProcessorEditor (NoiseGeneratorAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~NoiseGeneratorAudioProcessorEditor() override;

	static const int CANVAS_WIDTH = 1 + 3 + 3 + 3 + 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    NoiseGeneratorAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginLabel;

	TextModernRotarySlider m_typeSlider;
	
	ModernRotarySlider m_densitySlider;
	ModernRotarySlider m_wetSlider;
	ModernRotarySlider m_drySlider;	

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseGeneratorAudioProcessorEditor)
};
