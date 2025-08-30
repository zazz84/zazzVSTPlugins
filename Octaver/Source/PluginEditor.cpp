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
OctaverAudioProcessorEditor::OctaverAudioProcessorEditor (OctaverAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_noiseGateThresholdSlider			(vts,	OctaverAudioProcessor::paramsNames[0], OctaverAudioProcessor::paramsUnitNames[0], OctaverAudioProcessor::labelNames[0]),
	m_noiseGateSpeedSlider				(vts,	OctaverAudioProcessor::paramsNames[1], OctaverAudioProcessor::paramsUnitNames[1], OctaverAudioProcessor::labelNames[1]),
	m_envelopeSpeedSlider				(vts,	OctaverAudioProcessor::paramsNames[2], OctaverAudioProcessor::paramsUnitNames[2], OctaverAudioProcessor::labelNames[2]),
	m_generatorFrequencyMinSlider		(vts,	OctaverAudioProcessor::paramsNames[3], OctaverAudioProcessor::paramsUnitNames[3], OctaverAudioProcessor::labelNames[3]),
	m_generatorFrequencyMaxSlider		(vts,	OctaverAudioProcessor::paramsNames[4], OctaverAudioProcessor::paramsUnitNames[4], OctaverAudioProcessor::labelNames[4]),
	m_generatorSpeedSlider				(vts,	OctaverAudioProcessor::paramsNames[5], OctaverAudioProcessor::paramsUnitNames[5], OctaverAudioProcessor::labelNames[5]),
	m_generatorPitchSlider				(vts,	OctaverAudioProcessor::paramsNames[6], OctaverAudioProcessor::paramsUnitNames[6], OctaverAudioProcessor::labelNames[6]),
	m_generatorShapeSlider				(vts,	OctaverAudioProcessor::paramsNames[7], OctaverAudioProcessor::paramsUnitNames[7], OctaverAudioProcessor::labelNames[7]),
	m_generatorVolumeSlider				(vts,	OctaverAudioProcessor::paramsNames[8], OctaverAudioProcessor::paramsUnitNames[8], OctaverAudioProcessor::labelNames[8]),
	m_directVolumeSlider				(vts,	OctaverAudioProcessor::paramsNames[9], OctaverAudioProcessor::paramsUnitNames[9], OctaverAudioProcessor::labelNames[9]),
	m_noiseGateGroupLabel("Gate"),
	m_envelopeGroupLabel("Env"),
	m_frequencyGroupLabel("Frequency"),
	m_generatorGroupLabel("Generator"),
	m_pluginLabel("zazz::Octaver")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_noiseGateGroupLabel);
	addAndMakeVisible(m_envelopeGroupLabel);
	addAndMakeVisible(m_frequencyGroupLabel);
	addAndMakeVisible(m_generatorGroupLabel);

	addAndMakeVisible(m_noiseGateThresholdSlider);
	addAndMakeVisible(m_noiseGateSpeedSlider);
	addAndMakeVisible(m_envelopeSpeedSlider);
	addAndMakeVisible(m_generatorFrequencyMinSlider);
	addAndMakeVisible(m_generatorFrequencyMaxSlider);
	addAndMakeVisible(m_generatorSpeedSlider);
	addAndMakeVisible(m_generatorPitchSlider);
	addAndMakeVisible(m_generatorShapeSlider);
	addAndMakeVisible(m_generatorVolumeSlider);
	addAndMakeVisible(m_directVolumeSlider);

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

OctaverAudioProcessorEditor::~OctaverAudioProcessorEditor()
{
}

//==============================================================================
void OctaverAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void OctaverAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize6 = pixelSize3 + pixelSize3;
	const int pixelSize12 = pixelSize6 + pixelSize6;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);
	
	m_noiseGateGroupLabel.setSize(pixelSize6, pixelSize);
	m_envelopeGroupLabel.setSize(pixelSize3, pixelSize);
	m_frequencyGroupLabel.setSize(pixelSize6, pixelSize);
	m_generatorGroupLabel.setSize(pixelSize12, pixelSize);

	m_noiseGateThresholdSlider.setSize(pixelSize3, pixelSize4);
	m_noiseGateSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_envelopeSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_generatorFrequencyMinSlider.setSize(pixelSize3, pixelSize4);
	m_generatorFrequencyMaxSlider.setSize(pixelSize3, pixelSize4);
	m_generatorSpeedSlider.setSize(pixelSize3, pixelSize4);
	m_generatorPitchSlider.setSize(pixelSize3, pixelSize4);
	m_generatorShapeSlider.setSize(pixelSize3, pixelSize4);
	m_generatorVolumeSlider.setSize(pixelSize3, pixelSize4);
	m_directVolumeSlider.setSize(pixelSize3, pixelSize4);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
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
	const int column10 = column9 + pixelSize3;
	const int column11 = column10 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_noiseGateGroupLabel.setTopLeftPosition(column2, row2);
	m_envelopeGroupLabel.setTopLeftPosition(column4, row2);
	m_frequencyGroupLabel.setTopLeftPosition(column5, row2);
	m_generatorGroupLabel.setTopLeftPosition(column7, row2);

	m_noiseGateThresholdSlider.setTopLeftPosition			(column2, row3);
	m_noiseGateSpeedSlider.setTopLeftPosition				(column3, row3);
	m_envelopeSpeedSlider.setTopLeftPosition				(column4, row3);
	m_generatorFrequencyMinSlider.setTopLeftPosition		(column5, row3);
	m_generatorFrequencyMaxSlider.setTopLeftPosition		(column6, row3);
	m_generatorSpeedSlider.setTopLeftPosition				(column7, row3);
	m_generatorPitchSlider.setTopLeftPosition				(column8, row3);
	m_generatorShapeSlider.setTopLeftPosition				(column9, row3);
	m_generatorVolumeSlider.setTopLeftPosition				(column10, row3);
	m_directVolumeSlider.setTopLeftPosition					(column11, row3);
}