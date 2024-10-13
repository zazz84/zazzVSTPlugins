#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class SineWaveshaperAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    SineWaveshaperAudioProcessorEditor (SineWaveshaperAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~SineWaveshaperAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 5;
	static const int TYPE_BUTTON_GROUP = 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    SineWaveshaperAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	juce::TextButton button1{ "1" };
	juce::TextButton button2{ "2" };
	juce::TextButton button3{ "8x" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button1Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button2Attachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> button3Attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWaveshaperAudioProcessorEditor)
};
