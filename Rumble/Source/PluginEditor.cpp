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
RumbleAudioProcessorEditor::RumbleAudioProcessorEditor (RumbleAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_triggerGroupLabel("Trigger"),
	m_toneGroupLabel("Tone"),
	m_thresholdSlider	(vts,	RumbleAudioProcessor::paramsNames[0], RumbleAudioProcessor::paramsUnitNames[0], RumbleAudioProcessor::labelNames[0]),
	m_lenghtSlider		(vts,	RumbleAudioProcessor::paramsNames[1], RumbleAudioProcessor::paramsUnitNames[1], RumbleAudioProcessor::labelNames[1]),
	m_pitchSlider		(vts,	RumbleAudioProcessor::paramsNames[2], RumbleAudioProcessor::paramsUnitNames[2], RumbleAudioProcessor::labelNames[2]),
	m_delaySlider		(vts,	RumbleAudioProcessor::paramsNames[3], RumbleAudioProcessor::paramsUnitNames[3], RumbleAudioProcessor::labelNames[3]),
	m_amountSlider		(vts,	RumbleAudioProcessor::paramsNames[4], RumbleAudioProcessor::paramsUnitNames[4], RumbleAudioProcessor::labelNames[4]),
	m_volumeSlider		(vts,	RumbleAudioProcessor::paramsNames[5], RumbleAudioProcessor::paramsUnitNames[5], RumbleAudioProcessor::labelNames[5]),
	m_pluginLabel("zazz::Rumble")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_triggerGroupLabel);
	addAndMakeVisible(m_toneGroupLabel);

	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_lenghtSlider);
	addAndMakeVisible(m_pitchSlider);
	addAndMakeVisible(m_delaySlider);
	addAndMakeVisible(m_amountSlider);
	addAndMakeVisible(m_volumeSlider);

	setResizable(true, true);

	const int canvasWidth = CANVAS_WIDTH * 30;
	const int canvasHeight = CANVAS_HEIGHT * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	// Buttons
	m_soloButton.setLookAndFeel(&customLook);
	addAndMakeVisible(m_soloButton);
	m_soloButton.setColour(juce::TextButton::buttonColourId, lightColor);
	m_soloButton.setColour(juce::TextButton::buttonOnColourId, darkColor);
	m_soloButton.setClickingTogglesState(true);

	m_soloButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "S", m_soloButton));
}

RumbleAudioProcessorEditor::~RumbleAudioProcessorEditor()
{
}

//==============================================================================
void RumbleAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void RumbleAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize9 = pixelSize6 + pixelSize3;
	
	const int buttonSize = 90 * pixelSize / 100;
	const int buttonPixelOffset = 5 * pixelSize / 100;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_triggerGroupLabel.setSize(pixelSize6, pixelSize);
	m_toneGroupLabel.setSize(pixelSize9, pixelSize);

	m_thresholdSlider.setSize(pixelSize3, pixelSize4);
	m_lenghtSlider.setSize(pixelSize3, pixelSize4);
	m_pitchSlider.setSize(pixelSize3, pixelSize4);
	m_delaySlider.setSize(pixelSize3, pixelSize4);
	m_amountSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_soloButton.setSize(buttonSize, buttonSize);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize4 + pixelSize / 4;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_triggerGroupLabel.setTopLeftPosition(column2, row2);
	m_toneGroupLabel.setTopLeftPosition(column4, row2);

	m_thresholdSlider.setTopLeftPosition	(column2, row3);
	m_lenghtSlider.setTopLeftPosition		(column3, row3);
	m_pitchSlider.setTopLeftPosition		(column4, row3);
	m_delaySlider.setTopLeftPosition		(column5, row3);
	m_amountSlider.setTopLeftPosition		(column6, row3);
	m_volumeSlider.setTopLeftPosition		(column7, row3);

	m_soloButton.setTopLeftPosition(column5 + pixelSize + buttonPixelOffset, row4);
}