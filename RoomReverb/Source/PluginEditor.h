#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class RoomReverbAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    RoomReverbAudioProcessorEditor (RoomReverbAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~RoomReverbAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 15;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    RoomReverbAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

	juce::TextButton earlyReflectionsMuteButton{ "M" };
	juce::TextButton lateReflectionsMuteButton{ "M" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> earlyReflectionsMuteButtonAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lateReflectionsMuteButtonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoomReverbAudioProcessorEditor)
};
