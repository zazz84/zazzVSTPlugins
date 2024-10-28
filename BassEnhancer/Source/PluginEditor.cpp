#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.95f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.95f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.95f, 0.5f, 0.4f, 1.0f);

//==============================================================================
BassEnhancerAudioProcessorEditor::BassEnhancerAudioProcessorEditor (BassEnhancerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = BassEnhancerAudioProcessor::paramsNames[i];

		//Lable
		createLabel(label, text);
		addAndMakeVisible(label);

		//Slider
		createSlider(slider);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

BassEnhancerAudioProcessorEditor::~BassEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void BassEnhancerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void BassEnhancerAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);
}