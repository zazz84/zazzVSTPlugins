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
ClassBAmplifierAudioProcessorEditor::ClassBAmplifierAudioProcessorEditor (ClassBAmplifierAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_colorSlider(vts, ClassBAmplifierAudioProcessor::paramsNames[0], ClassBAmplifierAudioProcessor::paramsUnitNames[0], ClassBAmplifierAudioProcessor::labelNames[0]),
	m_frequencySlider(vts, ClassBAmplifierAudioProcessor::paramsNames[1], ClassBAmplifierAudioProcessor::paramsUnitNames[1], ClassBAmplifierAudioProcessor::labelNames[1]),
	m_driveSlider(vts, ClassBAmplifierAudioProcessor::paramsNames[2], ClassBAmplifierAudioProcessor::paramsUnitNames[2], ClassBAmplifierAudioProcessor::labelNames[2]),
	m_mixSlider(vts, ClassBAmplifierAudioProcessor::paramsNames[3], ClassBAmplifierAudioProcessor::paramsUnitNames[3], ClassBAmplifierAudioProcessor::labelNames[3]),
	m_volumeSlider(vts, ClassBAmplifierAudioProcessor::paramsNames[4], ClassBAmplifierAudioProcessor::paramsUnitNames[4], ClassBAmplifierAudioProcessor::labelNames[4]),

	m_pluginLabel("zazz::ClassBAmplifier")
{	
	addAndMakeVisible(m_colorSlider);
	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_driveSlider);
	addAndMakeVisible(m_mixSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_pluginLabel);

	m_driveSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

	setResizable(true, true);

	const int canvasWidth = (4 * 3 + 6 + 2 + 1) * 30;
	//const int canvasWidth = (2 * 3 + 6 + 2) * 30;
	const int canvasHeight = (2 + 8 + 1) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}
}

ClassBAmplifierAudioProcessorEditor::~ClassBAmplifierAudioProcessorEditor()
{
}

//==============================================================================
void ClassBAmplifierAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void ClassBAmplifierAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 21;
	//const int pixelSize = width / 14;
	const int pixelSize2 = pixelSize + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	const int sliderWidth = 3 * pixelSize;
	const int sliderHeight = 4 * pixelSize;

	const int bigSliderWidth = 2 * sliderWidth;
	const int bigSliderHeight = 2 * sliderHeight;

	m_colorSlider.setSize(sliderWidth, sliderHeight);
	m_frequencySlider.setSize(sliderWidth, sliderHeight);
	m_driveSlider.setSize(bigSliderWidth, bigSliderHeight);
	m_mixSlider.setSize(sliderWidth, sliderHeight);
	m_volumeSlider.setSize(sliderWidth, sliderHeight);

	// Set position
	m_pluginLabel.setTopLeftPosition(0, 0);

	const int sliderColumn1 = pixelSize;
	const int sliderColumn2 = sliderColumn1 + sliderWidth;
	const int sliderColumn3 = sliderColumn2 + sliderWidth + pixelSize / 2;
	const int sliderColumn4 = sliderColumn3 + bigSliderWidth + pixelSize / 2;
	const int sliderColumn5 = sliderColumn4 + sliderWidth;

	const int sliderRow1 = pixelSize2;
	const int sliderRow2 = sliderRow1 + sliderHeight;

	m_colorSlider.setTopLeftPosition(sliderColumn1, sliderRow2);
	m_frequencySlider.setTopLeftPosition(sliderColumn2, sliderRow2);
	m_driveSlider.setTopLeftPosition(sliderColumn3, sliderRow1 + pixelSize / 4);	// Trying to align sliders values
	m_mixSlider.setTopLeftPosition(sliderColumn4, sliderRow2);
	m_volumeSlider.setTopLeftPosition(sliderColumn5, sliderRow2);

	/*const int sliderColumn1 = pixelSize;
	const int sliderColumn2 = sliderColumn1 + sliderWidth;
	const int sliderColumn3 = sliderColumn2 + bigSliderWidth;

	const int sliderRow1 = pixelSize2;
	const int sliderRow2 = sliderRow1 + sliderHeight;

	m_colorSlider.setTopLeftPosition(sliderColumn1, sliderRow1);
	m_frequencySlider.setTopLeftPosition(sliderColumn1, sliderRow2);
	m_driveSlider.setTopLeftPosition(sliderColumn2, sliderRow1);
	m_mixSlider.setTopLeftPosition(sliderColumn3, sliderRow1);
	m_volumeSlider.setTopLeftPosition(sliderColumn3, sliderRow2);*/
}