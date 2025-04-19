#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ModernRotarySlider.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
	juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
	{
		return juce::Font(10.0f); // Custom font size
	}
};

//==============================================================================
class BiquadFilterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BiquadFilterAudioProcessorEditor (BiquadFilterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~BiquadFilterAudioProcessorEditor() override;

	static const int TYPE_BUTTON_GROUP = 1;
	static const int ALGORITHM_TYPE_BUTTON_GROUP = 2;

	static const int CANVAS_WIDTH = 1 + 2 + 1 + 2 + 1 + 3 + 3 + 3 + 3 + 3 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 4 + 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    BiquadFilterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	PluginNameComponent m_pluginNameComponent;

	GroupLabelComponent m_typeGroupLabel;
	GroupLabelComponent m_algorithmGroupLabel;

	ModernRotarySlider m_frequencySlider;
	ModernRotarySlider m_gainSlider;
	ModernRotarySlider m_QSlider;
	ModernRotarySlider m_mixSlider;
	ModernRotarySlider m_volumeSlider;

	CustomLookAndFeel customLook;

	juce::TextButton type1Button{ "LP" };
	juce::TextButton type2Button{ "HP" };
	juce::TextButton type3Button{ "AP" };
	juce::TextButton type4Button{ "BP" };
	juce::TextButton type5Button{ "N" };
	juce::TextButton type6Button{ "P" };
	juce::TextButton type7Button{ "LS" };
	juce::TextButton type8Button{ "HS" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button1Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button2Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button3Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button4Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button5Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button6Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button7Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button8Attachment;

	juce::TextButton algorithmType1Button{ "DF1" };
	juce::TextButton algorithmType2Button{ "DF2" };
	juce::TextButton algorithmType3Button{ "DF1T" };
	juce::TextButton algorithmType4Button{ "DF2T" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button9Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button10Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button11Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button12Attachment;

	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadFilterAudioProcessorEditor)
};
