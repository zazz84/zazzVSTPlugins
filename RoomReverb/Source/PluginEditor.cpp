#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR = juce::Colour::fromHSV(0.9f, 0.5f, 0.4f, 1.0f);

//==============================================================================
RoomReverbAudioProcessorEditor::RoomReverbAudioProcessorEditor (RoomReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		std::string text = RoomReverbAudioProcessor::paramsNames[i];

		//Lable
		createLabel(label, text);
		addAndMakeVisible(label);

		//Slider
		createSlider(slider);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	// Buttons
	createButton(earlyReflectionsMuteButton);
	createButton(lateReflectionsMuteButton);

	addAndMakeVisible(earlyReflectionsMuteButton);
	addAndMakeVisible(lateReflectionsMuteButton);

	earlyReflectionsMuteButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "EarlyReflectionsMute", earlyReflectionsMuteButton));
	lateReflectionsMuteButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "LateReflectionsMute", lateReflectionsMuteButton));

	// Canvas
	createCanvas(*this, N_SLIDERS);
}

RoomReverbAudioProcessorEditor::~RoomReverbAudioProcessorEditor()
{
}

//==============================================================================
void RoomReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);

	// Lines
	g.setColour(ZazzLookAndFeel::MEDIUM_COLOUR);
	const int width = (int)(getWidth() / N_SLIDERS);
	const int height = getHeight();
	
	g.drawVerticalLine(4 * width, 0, height);
	g.drawVerticalLine(9 * width, 0, height);
	g.drawVerticalLine(13 * width, 0, height);
}

void RoomReverbAudioProcessorEditor::resized()
{
	resize(*this, m_sliders, m_labels, N_SLIDERS);

	// Buttons
	const int height = getHeight();
	const float fonthHeight = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	const float buttonWidth = (float)height / (float)ZazzLookAndFeel::FONT_DIVISOR;
	int posY = height - (int)(1.8f * fonthHeight);

	const float width = getWidth() / N_SLIDERS;

	earlyReflectionsMuteButton.setBounds((int)(4.0f * width - 1.5f * buttonWidth), posY, buttonWidth, fonthHeight);
	lateReflectionsMuteButton.setBounds((int)(9.0f * width - 1.5f * buttonWidth), posY, buttonWidth, fonthHeight);
}