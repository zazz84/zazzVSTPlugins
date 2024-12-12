#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class CombFilterAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    CombFilterAudioProcessorEditor (CombFilterAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CombFilterAudioProcessorEditor() override;

	// GUI setup
	// Total number of sliders
	static const int N_SLIDERS = 6;
	// Total number of fows
	static const int N_ROWS = 1;
	// Number of sliders per row
	static const int SLIDERS[];
	// Number of slider width offset (from left) per row
	static const int COLUMN_OFFSET[];
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    CombFilterAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CombFilterAudioProcessorEditor)
};
