#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class DeharshAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    DeharshAudioProcessorEditor (DeharshAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~DeharshAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 5;
	static const int SLIDERS[];
	static const float COLUMN_OFFSET[];
	static const int N_ROWS = 1;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    DeharshAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_pluginName;
	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeharshAudioProcessorEditor)
};
