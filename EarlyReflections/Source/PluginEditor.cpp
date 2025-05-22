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
EarlyRefections1AudioProcessorEditor::EarlyRefections1AudioProcessorEditor (EarlyRefections1AudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_predelaySlider(vts,	EarlyRefections1AudioProcessor::paramsNames[0], EarlyRefections1AudioProcessor::paramsUnitNames[0], EarlyRefections1AudioProcessor::labelNames[0]),
	m_lenghtSlider(vts,		EarlyRefections1AudioProcessor::paramsNames[1], EarlyRefections1AudioProcessor::paramsUnitNames[1], EarlyRefections1AudioProcessor::labelNames[1]),
	m_decaySlider(vts,		EarlyRefections1AudioProcessor::paramsNames[2], EarlyRefections1AudioProcessor::paramsUnitNames[2], EarlyRefections1AudioProcessor::labelNames[2]),
	m_dampingSlider(vts,	EarlyRefections1AudioProcessor::paramsNames[3], EarlyRefections1AudioProcessor::paramsUnitNames[3], EarlyRefections1AudioProcessor::labelNames[3]),
	m_diffusionSlider(vts,	EarlyRefections1AudioProcessor::paramsNames[4], EarlyRefections1AudioProcessor::paramsUnitNames[4], EarlyRefections1AudioProcessor::labelNames[4]),
	m_widthSlider(vts,		EarlyRefections1AudioProcessor::paramsNames[5], EarlyRefections1AudioProcessor::paramsUnitNames[5], EarlyRefections1AudioProcessor::labelNames[5]),
	m_mixSlider(vts,		EarlyRefections1AudioProcessor::paramsNames[6], EarlyRefections1AudioProcessor::paramsUnitNames[6], EarlyRefections1AudioProcessor::labelNames[6]),
	m_volumeSlider(vts,		EarlyRefections1AudioProcessor::paramsNames[7], EarlyRefections1AudioProcessor::paramsUnitNames[7], EarlyRefections1AudioProcessor::labelNames[7]),
	m_pluginLabel("zazz::EarlyReflections1")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_predelaySlider);
	addAndMakeVisible(m_lenghtSlider);
	addAndMakeVisible(m_decaySlider);
	addAndMakeVisible(m_dampingSlider);
	addAndMakeVisible(m_diffusionSlider);
	addAndMakeVisible(m_widthSlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);

	m_mixSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);
	m_volumeSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

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

EarlyRefections1AudioProcessorEditor::~EarlyRefections1AudioProcessorEditor()
{
}

//==============================================================================
void EarlyRefections1AudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void EarlyRefections1AudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_predelaySlider.setSize(pixelSize3, pixelSize4);
	m_lenghtSlider.setSize(pixelSize3, pixelSize4);
	m_decaySlider.setSize(pixelSize3, pixelSize4);
	m_dampingSlider.setSize(pixelSize3, pixelSize4);
	m_diffusionSlider.setSize(pixelSize3, pixelSize4);
	m_widthSlider.setSize(pixelSize3, pixelSize4);
	m_mixSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = pixelSize2;

	const int column1 = 0;
	const int column2 = pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3 + pixelSize;
	const int column9 = column8 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_predelaySlider.setTopLeftPosition	(column2, row2);
	m_lenghtSlider.setTopLeftPosition	(column3, row2);
	m_decaySlider.setTopLeftPosition	(column4, row2);
	m_dampingSlider.setTopLeftPosition	(column5, row2);
	m_diffusionSlider.setTopLeftPosition(column6, row2);
	m_widthSlider.setTopLeftPosition	(column7, row2);
	m_mixSlider.setTopLeftPosition		(column8, row2);
	m_volumeSlider.setTopLeftPosition	(column9, row2);
}