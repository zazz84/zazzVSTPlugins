#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.6f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.6f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.6f, 0.5f, 0.4f, 1.0f);

//==============================================================================
DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = DistortionAudioProcessor::paramsNames[i];

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

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

//==============================================================================
void DistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);

	// Lines
	g.setColour(ZazzLookAndFeel::MEDIUM_COLOUR);
	const int width = (int)(getWidth() / N_SLIDERS);
	const int height = getHeight();

	g.drawVerticalLine(3 * width, 0, height);
	g.drawVerticalLine(5 * width, 0, height);
	g.drawVerticalLine(6 * width, 0, height);
}

void DistortionAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);
}