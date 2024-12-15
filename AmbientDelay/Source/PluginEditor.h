#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzLookAndFeel.h"
#include "../../../zazzVSTPlugins/Shared/GUI/ZazzAudioProcessorEditor.h"

//==============================================================================
class AmbientDelayAudioProcessorEditor : public juce::AudioProcessorEditor, public ZazzAudioProcessorEditor
{
public:
    AmbientDelayAudioProcessorEditor (AmbientDelayAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~AmbientDelayAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS = 23;
	static const int SLIDERS[];
	static const int COLUMN_OFFSET[];
	static const int N_ROWS = 3;
	
	//==============================================================================
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
	
protected:
    AmbientDelayAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_pluginName;
	juce::Label m_labels[N_SLIDERS] = {};
	juce::Slider m_sliders[N_SLIDERS] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmbientDelayAudioProcessorEditor)
};
