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
VolumeAudioProcessorEditor::VolumeAudioProcessorEditor (VolumeAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_volumeSlider(vts,	VolumeAudioProcessor::paramsNames[0], VolumeAudioProcessor::paramsUnitNames[0], VolumeAudioProcessor::labelNames[0]),
	m_pluginLabel("zazz::Volume"),
	m_minusTwelveButton	(vts, VolumeAudioProcessor::buttonNames[0]),
	m_minusSixButton	(vts, VolumeAudioProcessor::buttonNames[1]),
	m_plusSixButton		(vts, VolumeAudioProcessor::buttonNames[2]),
	m_plusTwelveButton	(vts, VolumeAudioProcessor::buttonNames[3])
{	
	addAndMakeVisible(m_minusTwelveButton);
	addAndMakeVisible(m_minusSixButton);
	addAndMakeVisible(m_plusSixButton);
	addAndMakeVisible(m_plusTwelveButton);

	m_minusTwelveButton.setBorder(BORDER);
	m_minusSixButton.setBorder(BORDER);
	m_plusSixButton.setBorder(BORDER);
	m_plusTwelveButton.setBorder(BORDER);

	addAndMakeVisible(m_pluginLabel);

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
}

VolumeAudioProcessorEditor::~VolumeAudioProcessorEditor()
{
}

//==============================================================================
void VolumeAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void VolumeAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSizeHalf = pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_minusTwelveButton.setSize(pixelSize, pixelSize);
	m_minusSixButton.setSize(pixelSize, pixelSize);
	m_plusSixButton.setSize(pixelSize, pixelSize);
	m_plusTwelveButton.setSize(pixelSize, pixelSize);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize4 + pixelSizeHalf;

	const int column1 = 0;
	const int column2 = column1 + pixelSize2;

	const int buttonColumn1 = pixelSize + pixelSizeHalf;
	const int buttonColumn2 = buttonColumn1 + pixelSize;
	const int buttonColumn3 = buttonColumn2 + pixelSize;
	const int buttonColumn4 = buttonColumn3 + pixelSize;


	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_volumeSlider.setTopLeftPosition	(column2, row2);

	m_minusTwelveButton.setTopLeftPosition	(buttonColumn1, row3);
	m_minusSixButton.setTopLeftPosition		(buttonColumn2, row3);
	m_plusSixButton.setTopLeftPosition		(buttonColumn3, row3);
	m_plusTwelveButton.setTopLeftPosition	(buttonColumn4, row3);


}