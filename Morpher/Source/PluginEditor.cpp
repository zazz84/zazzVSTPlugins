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
MorpherAudioProcessorEditor::MorpherAudioProcessorEditor (MorpherAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_inVolumeSlider(vts, MorpherAudioProcessor::paramsNames[MorpherAudioProcessor::Parameters::InVolume], MorpherAudioProcessor::paramsUnitNames[MorpherAudioProcessor::Parameters::InVolume], MorpherAudioProcessor::labelNames[MorpherAudioProcessor::Parameters::InVolume]),
	m_SCvolumeSlider(vts, MorpherAudioProcessor::paramsNames[MorpherAudioProcessor::Parameters::SCVolume], MorpherAudioProcessor::paramsUnitNames[MorpherAudioProcessor::Parameters::SCVolume], MorpherAudioProcessor::labelNames[MorpherAudioProcessor::Parameters::SCVolume]),
	m_morphSlider(vts, MorpherAudioProcessor::paramsNames[MorpherAudioProcessor::Parameters::Morph], MorpherAudioProcessor::paramsUnitNames[MorpherAudioProcessor::Parameters::Morph], MorpherAudioProcessor::labelNames[MorpherAudioProcessor::Parameters::Morph]),
	m_typeSlider(vts, MorpherAudioProcessor::paramsNames[MorpherAudioProcessor::Parameters::Type], MorpherAudioProcessor::paramsUnitNames[MorpherAudioProcessor::Parameters::Type], MorpherAudioProcessor::labelNames[MorpherAudioProcessor::Parameters::Type], {"Volume", "HP", "LP"}),
	m_volumeSlider(vts, MorpherAudioProcessor::paramsNames[MorpherAudioProcessor::Parameters::Volume], MorpherAudioProcessor::paramsUnitNames[MorpherAudioProcessor::Parameters::Volume], MorpherAudioProcessor::labelNames[MorpherAudioProcessor::Parameters::Volume]),
	m_pluginLabel("zazz::Morpher")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_inVolumeSlider);
	addAndMakeVisible(m_SCvolumeSlider);
	addAndMakeVisible(m_morphSlider);
	addAndMakeVisible(m_typeSlider);
	addAndMakeVisible(m_volumeSlider);

	m_morphSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

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

MorpherAudioProcessorEditor::~MorpherAudioProcessorEditor()
{
}

//==============================================================================
void MorpherAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void MorpherAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSizeHalf = pixelSize / 2;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize8 = pixelSize4 + pixelSize4;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_inVolumeSlider.setSize(pixelSize3, pixelSize4);
	m_SCvolumeSlider.setSize(pixelSize3, pixelSize4);
	m_morphSlider.setSize(pixelSize6, pixelSize8);
	m_typeSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize8;
	const int row4 = row3 + pixelSize4;

	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_morphSlider.setTopLeftPosition	(column2 + pixelSize + pixelSizeHalf, row2);

	m_inVolumeSlider.setTopLeftPosition	(column2, row3);
	m_volumeSlider.setTopLeftPosition	(column3, row3);
	m_SCvolumeSlider.setTopLeftPosition	(column4, row3);
	
	m_typeSlider.setTopLeftPosition		(column3, row4);
	
}