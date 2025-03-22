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
HarmonicEQAudioProcessorEditor::HarmonicEQAudioProcessorEditor (HarmonicEQAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_pluginNameComponent("zazz::HarmonicEQ"),
	m_frequencySlider	(vts, HarmonicEQAudioProcessor::paramsNames[0], HarmonicEQAudioProcessor::paramsUnitNames[0], HarmonicEQAudioProcessor::labelNames[0]),
	m_qSlider			(vts, HarmonicEQAudioProcessor::paramsNames[1], HarmonicEQAudioProcessor::paramsUnitNames[1], HarmonicEQAudioProcessor::labelNames[1]),
	m_gainSlider		(vts, HarmonicEQAudioProcessor::paramsNames[2], HarmonicEQAudioProcessor::paramsUnitNames[2], HarmonicEQAudioProcessor::labelNames[2]),
	m_stepSlider		(vts, HarmonicEQAudioProcessor::paramsNames[3], HarmonicEQAudioProcessor::paramsUnitNames[3], HarmonicEQAudioProcessor::labelNames[3]),
	m_coutSlider		(vts, HarmonicEQAudioProcessor::paramsNames[4], HarmonicEQAudioProcessor::paramsUnitNames[4], HarmonicEQAudioProcessor::labelNames[4]),
	m_slopeSlider		(vts, HarmonicEQAudioProcessor::paramsNames[5], HarmonicEQAudioProcessor::paramsUnitNames[5], HarmonicEQAudioProcessor::labelNames[5]),
	m_volumeSlider		(vts, HarmonicEQAudioProcessor::paramsNames[6], HarmonicEQAudioProcessor::paramsUnitNames[6], HarmonicEQAudioProcessor::labelNames[6])
{	
	addAndMakeVisible(m_pluginNameComponent);

	addAndMakeVisible(m_frequencySlider);
	addAndMakeVisible(m_qSlider);
	addAndMakeVisible(m_gainSlider);
	addAndMakeVisible(m_stepSlider);
	addAndMakeVisible(m_coutSlider);
	addAndMakeVisible(m_slopeSlider);
	addAndMakeVisible(m_volumeSlider);
	
	m_volumeSlider.setSliderType(ModernRotarySliderLookAndFeel::SliderType::Dots);

	// Set canvas
	setResizable(true, true);

	const int canvasWidth = (1 + 3 + 3 + 3 + 1 + 3 + 1) * 30;
	const int canvasHeight = (2 + 4 + 4 + 1) * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}
}

HarmonicEQAudioProcessorEditor::~HarmonicEQAudioProcessorEditor()
{
}

//==============================================================================
void HarmonicEQAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void HarmonicEQAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / 15;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize15 = pixelSize3 / 2;

	// Set size
	m_pluginNameComponent.setSize(width, pixelSize2);

	m_frequencySlider.setSize(pixelSize3, pixelSize4);
	m_qSlider.setSize(pixelSize3, pixelSize4);
	m_gainSlider.setSize(pixelSize3, pixelSize4);
	m_stepSlider.setSize(pixelSize3, pixelSize4);
	m_coutSlider.setSize(pixelSize3, pixelSize4);
	m_slopeSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	// Set position
	const int row1 = 0;
	const int row2 = pixelSize2;
	const int row3 = row2 + pixelSize4;

	const int column1 = pixelSize;
	const int column2 = column1 + pixelSize3;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize;

	m_pluginNameComponent.setTopLeftPosition(0, row1);

	m_frequencySlider.setTopLeftPosition(column1, row2);
	m_qSlider.setTopLeftPosition(column2, row2);
	m_gainSlider.setTopLeftPosition(column3, row2);

	m_stepSlider.setTopLeftPosition(column1, row3);
	m_coutSlider.setTopLeftPosition(column2, row3);
	m_slopeSlider.setTopLeftPosition(column3, row3);

	m_volumeSlider.setTopLeftPosition(column5, row2 + pixelSize15);
}