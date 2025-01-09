/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
const juce::Colour ZazzLookAndFeel::BACKGROUND_COLOR	= juce::Colour::fromRGB(90, 90, 100);
const juce::Colour ZazzLookAndFeel::KNOB_COLOR			= juce::Colour::fromRGB(70, 70, 80);
const juce::Colour ZazzLookAndFeel::KNOB_OUTLINE_COLOR	= juce::Colour::fromRGB(50, 50, 60);
const juce::Colour ZazzLookAndFeel::KNOB_HIGHLIGHT		= juce::Colour::fromRGB(55, 140, 255);
const juce::Colour ZazzLookAndFeel::MAIN_COLOR			= juce::Colour::fromRGB(240, 240, 255);

const int NoiseEnhancerAudioProcessorEditor::SLIDERS[] = { 5, 5, 4 };
const float NoiseEnhancerAudioProcessorEditor::COLUMN_OFFSET[] = { 0.0f, 0.0f, 0.5f };

//==============================================================================
NoiseEnhancerAudioProcessorEditor::NoiseEnhancerAudioProcessorEditor(NoiseEnhancerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_attackSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[0], NoiseEnhancerAudioProcessor::paramsUnitNames[0], NoiseEnhancerAudioProcessor::labelNames[0]),
	m_decaySlider(vts, NoiseEnhancerAudioProcessor::paramsNames[1], NoiseEnhancerAudioProcessor::paramsUnitNames[1], NoiseEnhancerAudioProcessor::labelNames[1]),
	m_sustainkSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[2], NoiseEnhancerAudioProcessor::paramsUnitNames[2], NoiseEnhancerAudioProcessor::labelNames[2]),
	m_sustainLevelSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[3], NoiseEnhancerAudioProcessor::paramsUnitNames[3], NoiseEnhancerAudioProcessor::labelNames[3]),
	m_releaseSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[4], NoiseEnhancerAudioProcessor::paramsUnitNames[4], NoiseEnhancerAudioProcessor::labelNames[4]),

	m_freqA0Slider(vts, NoiseEnhancerAudioProcessor::paramsNames[5], NoiseEnhancerAudioProcessor::paramsUnitNames[5], NoiseEnhancerAudioProcessor::labelNames[5]),
	m_freqA1Slider(vts, NoiseEnhancerAudioProcessor::paramsNames[6], NoiseEnhancerAudioProcessor::paramsUnitNames[6], NoiseEnhancerAudioProcessor::labelNames[6]),
	m_freqDSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[7], NoiseEnhancerAudioProcessor::paramsUnitNames[7], NoiseEnhancerAudioProcessor::labelNames[7]),
	m_freqSSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[8], NoiseEnhancerAudioProcessor::paramsUnitNames[8], NoiseEnhancerAudioProcessor::labelNames[8]),
	m_freqRSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[9], NoiseEnhancerAudioProcessor::paramsUnitNames[9], NoiseEnhancerAudioProcessor::labelNames[9]),

	m_frequencySlider(vts, NoiseEnhancerAudioProcessor::paramsNames[10], NoiseEnhancerAudioProcessor::paramsUnitNames[10], NoiseEnhancerAudioProcessor::labelNames[10]),
	m_thresholdSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[11], NoiseEnhancerAudioProcessor::paramsUnitNames[11], NoiseEnhancerAudioProcessor::labelNames[11]),
	m_amountSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[12], NoiseEnhancerAudioProcessor::paramsUnitNames[12], NoiseEnhancerAudioProcessor::labelNames[12]),
	m_volumeSlider(vts, NoiseEnhancerAudioProcessor::paramsNames[13], NoiseEnhancerAudioProcessor::paramsUnitNames[13], NoiseEnhancerAudioProcessor::labelNames[13]),

	m_amplitudeEnvelopeLabel("Amplitude Envelope"),
	m_frequencyEnvelopeLabel("Frequency Envelope"),
	m_triggerLabel("Trigger"),
	m_outputLabel("Output"),
	m_pluginLabel("Noise Enhancer"),

	m_triggerSoloButton(vts, "SOLO", "TriggerSolo"),
	m_noiseSoloButton(vts, "SOLO", "NoiseSolo")
{	
	addAndMakeVisible(m_attackSlider);
	addAndMakeVisible(m_decaySlider);
	addAndMakeVisible(m_sustainkSlider);
	addAndMakeVisible(m_sustainLevelSlider);
	addAndMakeVisible(m_releaseSlider);

	addAndMakeVisible(m_freqA0Slider);
	addAndMakeVisible(m_freqA1Slider);
	addAndMakeVisible(m_freqDSlider);
	addAndMakeVisible(m_freqSSlider);
	addAndMakeVisible(m_freqRSlider);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_amountSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_amplitudeEnvelopeLabel);
	addAndMakeVisible(m_frequencyEnvelopeLabel);
	addAndMakeVisible(m_triggerLabel);
	addAndMakeVisible(m_outputLabel);
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_triggerSoloButton);
	addAndMakeVisible(m_noiseSoloButton);

	setResizable(true, true);

	const int canvasWidth = 5 * 3 * 30;
	const int canvasHeight = (2 + 1 + 4 + 1 + 4 + 1 + 4 + 1) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}
}

NoiseEnhancerAudioProcessorEditor::~NoiseEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void NoiseEnhancerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(ZazzLookAndFeel::BACKGROUND_COLOR);
}

void NoiseEnhancerAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelWidth = width / 15;
	
	// Set size
	const int sliderWeidth = 3 * pixelWidth;
	const int sliderHeight = 4 * pixelWidth;

	m_attackSlider.setSize(sliderWeidth, sliderHeight);
	m_decaySlider.setSize(sliderWeidth, sliderHeight);
	m_sustainkSlider.setSize(sliderWeidth, sliderHeight);
	m_sustainLevelSlider.setSize(sliderWeidth, sliderHeight);
	m_releaseSlider.setSize(sliderWeidth, sliderHeight);

	m_freqA0Slider.setSize(sliderWeidth, sliderHeight);
	m_freqA1Slider.setSize(sliderWeidth, sliderHeight);
	m_freqDSlider.setSize(sliderWeidth, sliderHeight);
	m_freqSSlider.setSize(sliderWeidth, sliderHeight);
	m_freqRSlider.setSize(sliderWeidth, sliderHeight);

	m_frequencySlider.setSize(sliderWeidth, sliderHeight);
	m_thresholdSlider.setSize(sliderWeidth, sliderHeight);
	m_amountSlider.setSize(sliderWeidth, sliderHeight);
	m_volumeSlider.setSize(sliderWeidth, sliderHeight);

	m_amplitudeEnvelopeLabel.setSize(width, pixelWidth);
	m_frequencyEnvelopeLabel.setSize(width, pixelWidth);
	m_triggerLabel.setSize(6 * pixelWidth, pixelWidth);
	m_outputLabel.setSize(6 * pixelWidth, pixelWidth);
	
	m_pluginLabel.setSize(width, 2 * pixelWidth);

	m_triggerSoloButton.setSize(sliderWeidth, pixelWidth);
	m_noiseSoloButton.setSize(sliderWeidth, pixelWidth);

	// Set position

	const int column2 = 3 * pixelWidth;
	const int column3 = column2 + column2;
	const int column4 = column3 + column2;
	const int column5 = column4 + column2;

	m_pluginLabel.setTopLeftPosition(0, 0);

	const int glRow1 = pixelWidth + pixelWidth;
	m_amplitudeEnvelopeLabel.setTopLeftPosition(0, glRow1);

	const int row1 = glRow1 + pixelWidth;
	m_attackSlider.setTopLeftPosition(0, row1);
	m_decaySlider.setTopLeftPosition(column2, row1);
	m_sustainkSlider.setTopLeftPosition(column3, row1);
	m_sustainLevelSlider.setTopLeftPosition(column4, row1);
	m_releaseSlider.setTopLeftPosition(column5, row1);

	const int glRow2 = row1 + 4 * pixelWidth;
	m_frequencyEnvelopeLabel.setTopLeftPosition(0, glRow2);

	const int row2 = glRow2 + pixelWidth;
	m_freqA0Slider.setTopLeftPosition(0, row2);
	m_freqA1Slider.setTopLeftPosition(column2, row2);
	m_freqDSlider.setTopLeftPosition(column3, row2);
	m_freqSSlider.setTopLeftPosition(column4, row2);
	m_freqRSlider.setTopLeftPosition(column5, row2);

	const int glRow3 = row2 + 4 * pixelWidth;

	m_triggerLabel.setTopLeftPosition(3 * pixelWidth / 2, glRow3);
	m_outputLabel.setTopLeftPosition(6 * pixelWidth + 3 * pixelWidth / 2, glRow3);

	const int row3 = glRow3 + pixelWidth;
	m_frequencySlider.setTopLeftPosition(3 * pixelWidth / 2, row3);
	m_thresholdSlider.setTopLeftPosition(column2 + 3 * pixelWidth / 2, row3);
	m_amountSlider.setTopLeftPosition(column3 + 3 * pixelWidth / 2, row3);
	m_volumeSlider.setTopLeftPosition(column4 + 3 * pixelWidth / 2, row3);

	const int bRow = row3 + 4 * pixelWidth;
	m_triggerSoloButton.setTopLeftPosition(3 * pixelWidth / 2, bRow);
	m_noiseSoloButton.setTopLeftPosition(6 * pixelWidth + 3 * pixelWidth / 2, bRow);
}