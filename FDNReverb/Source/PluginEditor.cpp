#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::LIGHT_COLOUR  = juce::Colour::fromHSV(0.40f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel::MEDIUM_COLOUR = juce::Colour::fromHSV(0.40f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel::DARK_COLOUR   = juce::Colour::fromHSV(0.40f, 0.5f, 0.4f, 1.0f);

const juce::Colour ZazzLookAndFeel_V2::LIGHT_COLOUR = juce::Colour::fromHSV(0.40f, 0.5f, 0.6f, 1.0f);
const juce::Colour ZazzLookAndFeel_V2::MEDIUM_COLOUR = juce::Colour::fromHSV(0.40f, 0.5f, 0.5f, 1.0f);
const juce::Colour ZazzLookAndFeel_V2::DARK_COLOUR = juce::Colour::fromHSV(0.40f, 0.5f, 0.4f, 1.0f);

const int FDNReverbAudioProcessorEditor::SLIDERS[] = { 6, 6, 4 };
const int FDNReverbAudioProcessorEditor::COLUMN_OFFSET[] = { 0, 0, 1 };

//==============================================================================
FDNReverbAudioProcessorEditor::FDNReverbAudioProcessorEditor (FDNReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	// Lables and sliders
	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];
		const std::string text = FDNReverbAudioProcessor::paramsNames[i];
		const std::string unit = FDNReverbAudioProcessor::paramsUnitNames[i];

		createSliderWithLabel(slider, label, text, unit);
		addAndMakeVisible(label);
		addAndMakeVisible(slider);

		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, text, slider));
	}

	createCanvasMultiRow(*this, SLIDERS, N_ROWS);
}

FDNReverbAudioProcessorEditor::~FDNReverbAudioProcessorEditor()
{
}

//==============================================================================
void FDNReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::LIGHT_COLOUR);

	// Lines
	g.setColour(ZazzLookAndFeel::MEDIUM_COLOUR);
	const float width = (float)getWidth();
	const int height = getHeight() / N_ROWS;

	g.setColour(ZazzLookAndFeel::MEDIUM_COLOUR);
	g.drawHorizontalLine(1 * height, 0.0f, width);
	g.drawHorizontalLine(2 * height, 0.0f, width);
	g.drawHorizontalLine(3 * height, 0.0f, width);
}

void FDNReverbAudioProcessorEditor::resized()
{
	//resize(*this, m_sliders, m_labels, N_SLIDERS);
	resizeMultiRow(*this, m_sliders, m_labels, SLIDERS, COLUMN_OFFSET, N_ROWS);
}