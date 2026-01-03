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
#define CREATE_PARAM_SLIDER(sliderName, paramEnum) \
    sliderName( \
        vts, \
        VoiceFilterAudioProcessor::paramsNames[VoiceFilterAudioProcessor::Parameters::paramEnum], \
        VoiceFilterAudioProcessor::paramsUnitNames[VoiceFilterAudioProcessor::Parameters::paramEnum], \
        VoiceFilterAudioProcessor::labelNames[VoiceFilterAudioProcessor::Parameters::paramEnum] \
    )

//==============================================================================
VoiceFilterAudioProcessorEditor::VoiceFilterAudioProcessorEditor (VoiceFilterAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	CREATE_PARAM_SLIDER(m_gain1Slider, Gain1),
	CREATE_PARAM_SLIDER(m_gain2Slider, Gain2),
	CREATE_PARAM_SLIDER(m_gain3Slider, Gain3),
	CREATE_PARAM_SLIDER(m_gain4Slider, Gain4),
	CREATE_PARAM_SLIDER(m_gain5Slider, Gain5),
	CREATE_PARAM_SLIDER(m_volumeSlider, Volume),
	m_pluginLabel("zazz::VoiceFilter")
{	
	addAndMakeVisible(m_pluginLabel);

	addAndMakeVisible(m_gain1Slider);
	addAndMakeVisible(m_gain2Slider);
	addAndMakeVisible(m_gain3Slider);
	addAndMakeVisible(m_gain4Slider);
	addAndMakeVisible(m_gain5Slider);

	addAndMakeVisible(m_volumeSlider);
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

VoiceFilterAudioProcessorEditor::~VoiceFilterAudioProcessorEditor()
{
}

//==============================================================================
void VoiceFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void VoiceFilterAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);

	m_gain1Slider.setSize(pixelSize3, pixelSize4);
	m_gain2Slider.setSize(pixelSize3, pixelSize4);
	m_gain3Slider.setSize(pixelSize3, pixelSize4);
	m_gain4Slider.setSize(pixelSize3, pixelSize4);
	m_gain5Slider.setSize(pixelSize3, pixelSize4);
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

	m_gain1Slider.setTopLeftPosition	(column2, row2);
	m_gain2Slider.setTopLeftPosition	(column3, row2);
	m_gain3Slider.setTopLeftPosition	(column4, row2);
	m_gain4Slider.setTopLeftPosition	(column5, row2);
	m_gain5Slider.setTopLeftPosition	(column6, row2);
	m_volumeSlider.setTopLeftPosition	(column7, row2);
}