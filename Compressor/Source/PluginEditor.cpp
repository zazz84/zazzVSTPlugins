#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.65f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.65f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.65f, 0.5f, 0.4f, 1.0f);

//==============================================================================
CompressorAudioProcessorEditor::CompressorAudioProcessorEditor (CompressorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = CompressorAudioProcessor::paramsNames[i];

		//Lable
		createLabel(label, text);
		addAndMakeVisible(label);

		//Slider
		createSlider(slider);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	// Buttons
	createButton(button1, TYPE_BUTTON_GROUP);
	createButton(button2, TYPE_BUTTON_GROUP);
	createButton(button3, STYLE_BUTTON_GROUP);
	createButton(button4, STYLE_BUTTON_GROUP);
	createButton(button5, DETECTION_BUTTON_GROUP);
	createButton(button6, DETECTION_BUTTON_GROUP);
	createButton(button7, STYLE_BUTTON_GROUP);
	createButton(button8, STYLE_BUTTON_GROUP);

	addAndMakeVisible(button1);
	addAndMakeVisible(button2);
	addAndMakeVisible(button3);
	addAndMakeVisible(button4);
	addAndMakeVisible(button5);
	addAndMakeVisible(button6);
	addAndMakeVisible(button7);
	addAndMakeVisible(button8);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LOG", button1));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LIN", button2));
	button3Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "VCA", button3));
	button4Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Opto", button4));
	button5Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Peak", button5));
	button6Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "RMS", button6));
	button7Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Slew", button7));
	button8Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Dual", button8));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
}

//==============================================================================
void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void CompressorAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);

	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const float buttonWidth = 1.8f * (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	int posY = height - (int)(1.2f * fonthHeight);

	const float width = (float)getWidth() / (float)N_SLIDERS;

	button1.setBounds((int)(3.0f * width - 1.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
	button2.setBounds((int)(3.0f * width + 0.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);

	button3.setBounds((int)(2.0f * width - 1.0f * buttonWidth), posY - (int)fonthHeight, (int)buttonWidth, (int)fonthHeight);
	button4.setBounds((int)(2.0f * width + 0.0f * buttonWidth), posY - (int)fonthHeight, (int)buttonWidth, (int)fonthHeight);

	button7.setBounds((int)(2.0f * width - 1.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
	button8.setBounds((int)(2.0f * width + 0.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);

	button5.setBounds((int)(4.0f * width - 1.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
	button6.setBounds((int)(4.0f * width + 0.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
}