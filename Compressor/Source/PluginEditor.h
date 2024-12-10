#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel_OLD.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor_OLD.h"

//==============================================================================
class CompressorAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    CompressorAudioProcessorEditor (CompressorAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CompressorAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 6;
	static const int TYPE_BUTTON_GROUP = 1;
	static const int STYLE_BUTTON_GROUP = 2;
	static const int DETECTION_BUTTON_GROUP = 3;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    CompressorAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	juce::TextButton button1{ "LOG" };
	juce::TextButton button2{ "LIN" };
	juce::TextButton button3{ "VCA" };
	juce::TextButton button4{ "Opto" };
	juce::TextButton button5{ "Peak" };
	juce::TextButton button6{ "RMS" };
	juce::TextButton button7{ "Slew" };
	juce::TextButton button8{ "Dual" };


	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button1Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button2Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button3Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button4Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button5Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button6Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button7Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button8Attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessorEditor)
};
