/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
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
BitCrusher2AudioProcessorEditor::BitCrusher2AudioProcessorEditor (BitCrusher2AudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_bitDepthSlider(vts, BitCrusher2AudioProcessor::m_parametersDescritpion[BitCrusher2AudioProcessor::Parameters::BitDepth]),
	m_driveSlider(vts, BitCrusher2AudioProcessor::m_parametersDescritpion[BitCrusher2AudioProcessor::Parameters::Drive]),
	m_downsampleSlider(vts, BitCrusher2AudioProcessor::m_parametersDescritpion[BitCrusher2AudioProcessor::Parameters::Downsample]),
	m_mixSlider(vts, BitCrusher2AudioProcessor::m_parametersDescritpion[BitCrusher2AudioProcessor::Parameters::Mix]),
	m_volumeSlider(vts, BitCrusher2AudioProcessor::m_parametersDescritpion[BitCrusher2AudioProcessor::Parameters::Volume]),
	m_pluginLabel("zazz::BitCrusher2")
{
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_bitDepthSlider);
	addAndMakeVisible(m_driveSlider);
	addAndMakeVisible(m_downsampleSlider);
	addAndMakeVisible(m_mixSlider);
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

BitCrusher2AudioProcessorEditor::~BitCrusher2AudioProcessorEditor()
{
}

//==============================================================================
void BitCrusher2AudioProcessorEditor::paint (juce::Graphics& g)
{	
	g.fillAll(darkColor);
}

void BitCrusher2AudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_bitDepthSlider.setSize(pixelSize3, pixelSize4);
	m_driveSlider.setSize(pixelSize3, pixelSize4);
	m_downsampleSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_bitDepthSlider.setTopLeftPosition(column2, row2);
	m_driveSlider.setTopLeftPosition(column3, row2);
	m_downsampleSlider.setTopLeftPosition(column4, row2);
	m_mixSlider.setTopLeftPosition(column5, row2);
	m_volumeSlider.setTopLeftPosition(column6, row2);
}