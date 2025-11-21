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
ResynthesizerAudioProcessorEditor::ResynthesizerAudioProcessorEditor(ResynthesizerAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
	: AudioProcessorEditor(&p),
	audioProcessor(p),
	valueTreeState(vts),
	m_frequency1Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency1], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency1], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency1]),
	m_frequency2Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency2], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency2], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency2]),
	m_frequency3Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency3], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency3], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency3]),
	m_frequency4Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency4], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency4], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency4]),
	m_frequency5Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency5], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency5], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency5]),
	m_frequency6Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency6], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency6], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency6]),
	m_frequency7Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency7], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency7], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency7]),
	m_frequency8Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Frequency8], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Frequency8], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Frequency8]),
	m_volume1Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume1], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume1], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume1]),
	m_volume2Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume2], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume2], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume2]),
	m_volume3Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume3], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume3], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume3]),
	m_volume4Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume4], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume4], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume4]),
	m_volume5Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume5], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume5], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume5]),
	m_volume6Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume6], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume6], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume6]),
	m_volume7Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume7], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume7], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume7]),
	m_volume8Slider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume8], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume8], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume8]),
	m_styleSlider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Style], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Style], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Style], {"Pure", "Noise"}),
	m_shapeSlider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Shape], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Shape], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Shape]),
	m_factorSlider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Factor], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Factor], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Factor]),
	m_minimumStepSlider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::MinimumStep], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::MinimumStep], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::MinimumStep]),
	m_volumeSlider(vts, ResynthesizerAudioProcessor::paramsNames[ResynthesizerAudioProcessor::Parameters::Volume], ResynthesizerAudioProcessor::paramsUnitNames[ResynthesizerAudioProcessor::Parameters::Volume], ResynthesizerAudioProcessor::labelNames[ResynthesizerAudioProcessor::Parameters::Volume]),
	m_learnButton(vts, "Learn"),
	m_frequencyGroupLabel("Frequency"),
	m_volumeGroupLabel("Volume"),
	m_pluginLabel("zazz::Resynthesizer")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_frequency1Slider);
	addAndMakeVisible(m_frequency2Slider);
	addAndMakeVisible(m_frequency3Slider);
	addAndMakeVisible(m_frequency4Slider);
	addAndMakeVisible(m_frequency5Slider);
	addAndMakeVisible(m_frequency6Slider);
	addAndMakeVisible(m_frequency7Slider);
	addAndMakeVisible(m_frequency8Slider);

	addAndMakeVisible(m_volume1Slider);
	addAndMakeVisible(m_volume2Slider);
	addAndMakeVisible(m_volume3Slider);
	addAndMakeVisible(m_volume4Slider);
	addAndMakeVisible(m_volume5Slider);
	addAndMakeVisible(m_volume6Slider);
	addAndMakeVisible(m_volume7Slider);
	addAndMakeVisible(m_volume8Slider);

	addAndMakeVisible(m_styleSlider);
	addAndMakeVisible(m_factorSlider);
	addAndMakeVisible(m_shapeSlider);
	addAndMakeVisible(m_minimumStepSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_learnButton);

	addAndMakeVisible(m_frequencyGroupLabel);
	addAndMakeVisible(m_volumeGroupLabel);

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

ResynthesizerAudioProcessorEditor::~ResynthesizerAudioProcessorEditor()
{
}

//==============================================================================
void ResynthesizerAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void ResynthesizerAudioProcessorEditor::resized()
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
	m_frequencyGroupLabel.setSize(width - pixelSize2, pixelSize);
	m_volumeGroupLabel.setSize(width - pixelSize2, pixelSize);

	m_frequency1Slider.setSize(pixelSize3, pixelSize4);
	m_frequency2Slider.setSize(pixelSize3, pixelSize4);
	m_frequency3Slider.setSize(pixelSize3, pixelSize4);
	m_frequency4Slider.setSize(pixelSize3, pixelSize4);
	m_frequency5Slider.setSize(pixelSize3, pixelSize4);
	m_frequency6Slider.setSize(pixelSize3, pixelSize4);
	m_frequency7Slider.setSize(pixelSize3, pixelSize4);
	m_frequency8Slider.setSize(pixelSize3, pixelSize4);
	
	m_volume1Slider.setSize(pixelSize3, pixelSize4);
	m_volume2Slider.setSize(pixelSize3, pixelSize4);
	m_volume3Slider.setSize(pixelSize3, pixelSize4);
	m_volume4Slider.setSize(pixelSize3, pixelSize4);
	m_volume5Slider.setSize(pixelSize3, pixelSize4);
	m_volume6Slider.setSize(pixelSize3, pixelSize4);
	m_volume7Slider.setSize(pixelSize3, pixelSize4);
	m_volume8Slider.setSize(pixelSize3, pixelSize4);
	
	m_styleSlider.setSize(pixelSize3, pixelSize4);
	m_shapeSlider.setSize(pixelSize3, pixelSize4);
	m_factorSlider.setSize(pixelSize3, pixelSize4);
	m_minimumStepSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_learnButton.setSize(pixelSize3 + pixelSize3, pixelSize);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize - pixelSizeHalf;
	const int row4 = row3 + pixelSize4 + pixelSizeHalf;
	const int row5 = row4 + pixelSize - pixelSizeHalf;
	const int row6 = row5 + pixelSize4 + pixelSizeHalf;
	const int row7 = row6 + pixelSize4;

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

	m_frequencyGroupLabel.setTopLeftPosition(column2, row2);

	m_frequency1Slider.setTopLeftPosition(column2, row3);
	m_frequency2Slider.setTopLeftPosition(column3, row3);
	m_frequency3Slider.setTopLeftPosition(column4, row3);
	m_frequency4Slider.setTopLeftPosition(column5, row3);
	m_frequency5Slider.setTopLeftPosition(column6, row3);
	m_frequency6Slider.setTopLeftPosition(column7, row3);
	m_frequency7Slider.setTopLeftPosition(column8, row3);
	m_frequency8Slider.setTopLeftPosition(column9, row3);

	m_volumeGroupLabel.setTopLeftPosition(column2, row4);

	m_volume1Slider.setTopLeftPosition(column2, row5);
	m_volume2Slider.setTopLeftPosition(column3, row5);
	m_volume3Slider.setTopLeftPosition(column4, row5);
	m_volume4Slider.setTopLeftPosition(column5, row5);
	m_volume5Slider.setTopLeftPosition(column6, row5);
	m_volume6Slider.setTopLeftPosition(column7, row5);
	m_volume7Slider.setTopLeftPosition(column8, row5);
	m_volume8Slider.setTopLeftPosition(column9, row5);

	m_styleSlider.setTopLeftPosition		(column2, row6);
	m_shapeSlider.setTopLeftPosition		(column3, row6);
	m_factorSlider.setTopLeftPosition		(column4, row6);
	m_minimumStepSlider.setTopLeftPosition	(column6, row6);
	m_volumeSlider.setTopLeftPosition		(column7, row6);

	m_learnButton.setTopLeftPosition	(column5, row7);
}