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
RadioRackAudioProcessorEditor::RadioRackAudioProcessorEditor (RadioRackAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_gateLabel("Gate"),
	m_compressorLabel("Comp"),
	m_distortionLabel("Distortion"),
	m_speakerLabel("Speaker"),
	m_thresholdSlider	(vts,	RadioRackAudioProcessor::paramsNames[0], RadioRackAudioProcessor::paramsUnitNames[0], RadioRackAudioProcessor::labelNames[0]),
	m_compressionSlider	(vts,	RadioRackAudioProcessor::paramsNames[1], RadioRackAudioProcessor::paramsUnitNames[1], RadioRackAudioProcessor::labelNames[1]),
	m_driveSlider		(vts,	RadioRackAudioProcessor::paramsNames[2], RadioRackAudioProcessor::paramsUnitNames[2], RadioRackAudioProcessor::labelNames[2]),
	m_splitSlider		(vts,	RadioRackAudioProcessor::paramsNames[3], RadioRackAudioProcessor::paramsUnitNames[3], RadioRackAudioProcessor::labelNames[3]),
	m_speakerType		(vts,	RadioRackAudioProcessor::paramsNames[4], RadioRackAudioProcessor::paramsUnitNames[4], RadioRackAudioProcessor::labelNames[4]),
	m_speakerResonance	(vts,	RadioRackAudioProcessor::paramsNames[5], RadioRackAudioProcessor::paramsUnitNames[5], RadioRackAudioProcessor::labelNames[5]),
	m_speakerSize		(vts,	RadioRackAudioProcessor::paramsNames[6], RadioRackAudioProcessor::paramsUnitNames[6], RadioRackAudioProcessor::labelNames[6]),
	m_volumeSlider		(vts,	RadioRackAudioProcessor::paramsNames[7], RadioRackAudioProcessor::paramsUnitNames[7], RadioRackAudioProcessor::labelNames[7]),
	m_pluginLabel("zazz::RadioRack")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_gateLabel);
	addAndMakeVisible(m_compressorLabel);
	addAndMakeVisible(m_distortionLabel);
	addAndMakeVisible(m_speakerLabel);

	addAndMakeVisible(m_thresholdSlider);
	addAndMakeVisible(m_compressionSlider);
	addAndMakeVisible(m_driveSlider);
	addAndMakeVisible(m_splitSlider);
	addAndMakeVisible(m_speakerType);
	addAndMakeVisible(m_speakerResonance);
	addAndMakeVisible(m_speakerSize);
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

RadioRackAudioProcessorEditor::~RadioRackAudioProcessorEditor()
{
}

//==============================================================================
void RadioRackAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void RadioRackAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize9 = pixelSize6 + pixelSize3;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_gateLabel.setSize			(pixelSize3, pixelSize);
	m_compressorLabel.setSize	(pixelSize3, pixelSize);
	m_distortionLabel.setSize	(pixelSize6, pixelSize);
	m_speakerLabel.setSize		(pixelSize9, pixelSize);

	m_thresholdSlider.setSize(pixelSize3, pixelSize4);
	m_compressionSlider.setSize(pixelSize3, pixelSize4);
	m_driveSlider.setSize(pixelSize3, pixelSize4);
	m_splitSlider.setSize(pixelSize3, pixelSize4);
	m_speakerType.setSize(pixelSize3, pixelSize4);
	m_speakerResonance.setSize(pixelSize3, pixelSize4);
	m_speakerSize.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3;
	const int column9 = column8 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_gateLabel.setTopLeftPosition			(column2, row2);
	m_compressorLabel.setTopLeftPosition	(column3, row2);
	m_distortionLabel.setTopLeftPosition	(column4, row2);
	m_speakerLabel.setTopLeftPosition		(column6, row2);

	m_thresholdSlider.setTopLeftPosition	(column2, row3);
	m_compressionSlider.setTopLeftPosition	(column3, row3);
	m_driveSlider.setTopLeftPosition		(column4, row3);
	m_splitSlider.setTopLeftPosition		(column5, row3);
	m_speakerType.setTopLeftPosition		(column6, row3);
	m_speakerResonance.setTopLeftPosition	(column7, row3);
	m_speakerSize.setTopLeftPosition		(column8, row3);
	m_volumeSlider.setTopLeftPosition		(column9, row3);
}