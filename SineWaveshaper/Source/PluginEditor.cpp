#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.82f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.82f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.82f, 0.5f, 0.4f, 1.0f);

//==============================================================================
SineWaveshaperAudioProcessorEditor::SineWaveshaperAudioProcessorEditor (SineWaveshaperAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = SineWaveshaperAudioProcessor::paramsNames[i];

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
	createButton(button3);

	addAndMakeVisible(button1);
	addAndMakeVisible(button2);
	addAndMakeVisible(button3);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "1", button1));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "2", button2));
	button3Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "32x", button3));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

SineWaveshaperAudioProcessorEditor::~SineWaveshaperAudioProcessorEditor()
{
}

//==============================================================================
void SineWaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);
}

void SineWaveshaperAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);

	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const float buttonWidth = 1.8f * (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	int posY = height - (int)(1.2f * fonthHeight);

	const float canvasWidth = (float)getWidth();
	const float width = canvasWidth / (float)N_SLIDERS;

	button1.setBounds((int)(1.0f * width - 1.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
	button2.setBounds((int)(1.0f * width + 0.0f * buttonWidth), posY, (int)buttonWidth, (int)fonthHeight);
	
	button3.setBounds((int)(canvasWidth - buttonWidth), 0, (int)buttonWidth, (int)fonthHeight);
}