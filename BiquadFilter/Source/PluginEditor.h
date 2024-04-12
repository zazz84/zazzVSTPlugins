#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class BiquadFilterAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    BiquadFilterAudioProcessorEditor (BiquadFilterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~BiquadFilterAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 5;
	static const int TYPE_BUTTON_GROUP = 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    BiquadFilterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	juce::TextButton type1Button{ "LP" };
	juce::TextButton type2Button{ "HP" };
	juce::TextButton type3Button{ "BP1" };
	juce::TextButton type4Button{ "BP2" };
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BiquadFilterAudioProcessorEditor)
};
