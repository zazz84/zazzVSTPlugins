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
SpectrumMatchFFTAudioProcessorEditor::SpectrumMatchFFTAudioProcessorEditor (SpectrumMatchFFTAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	valueTreeState(vts),
	m_lowPassFilterSlider	(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::LowPassFilter], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::LowPassFilter], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::LowPassFilter]),
	m_highPassFilterSlider	(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::HighPassFilter], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::HighPassFilter], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::HighPassFilter]),
	m_frequencyShiftSlider	(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::FrequencyShift], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::FrequencyShift], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::FrequencyShift]),
	m_resolutionSlider		(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::Resolution], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::Resolution], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::Resolution]),
	m_ammountSlider			(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::Ammount], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::Ammount], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::Ammount]),
	m_volumeSlider			(vts,	SpectrumMatchFFTAudioProcessor::paramsNames[SpectrumMatchFFTAudioProcessor::Parameters::Volume], SpectrumMatchFFTAudioProcessor::paramsUnitNames[SpectrumMatchFFTAudioProcessor::Parameters::Volume], SpectrumMatchFFTAudioProcessor::labelNames[SpectrumMatchFFTAudioProcessor::Parameters::Volume]),
	m_sourceSpectrumButton(vts, "Learn Source"),
	m_targetSpectrumButton(vts, "Learn Target"),
	m_spectrumGroupLabel("Source + Target"),
	m_spectrumDiffGroupLabel("Filter Curve"),
	m_pluginLabel("zazz::SpectrumMatchFFT")
{	
	addAndMakeVisible(m_pluginLabel);
	
	addAndMakeVisible(m_spectrumGroupLabel);
	addAndMakeVisible(m_spectrumDiffGroupLabel);


	addAndMakeVisible(m_sourceTargetSpectrumCurveComponent);
	addAndMakeVisible(m_filterSpectrumComponent);

	addAndMakeVisible(m_lowPassFilterSlider);
	addAndMakeVisible(m_highPassFilterSlider);
	addAndMakeVisible(m_frequencyShiftSlider);
	addAndMakeVisible(m_resolutionSlider);
	addAndMakeVisible(m_ammountSlider);
	addAndMakeVisible(m_volumeSlider);

	addAndMakeVisible(m_sourceSpectrumButton);
	addAndMakeVisible(m_targetSpectrumButton);

	m_sourceSpectrumButton.setBorder(10);
	m_targetSpectrumButton.setBorder(10);

	/*m_sourceSpectrumButton.m_textButton.onClick = [this]
	{

	};

	m_targetSpectrumButton.m_textButton.onClick = [this]
	{

	};*/

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

	m_sourceTargetSpectrumCurveComponent.setFFTSize(audioProcessor.getFFTSize());
	m_sourceTargetSpectrumCurveComponent.setSampleRate(audioProcessor.getSampleRate());
	m_sourceTargetSpectrumCurveComponent.setMagnitudeRangeDb(-80.0f, -12.0f);

	m_filterSpectrumComponent.setFFTSize(audioProcessor.getFFTSize());
	m_filterSpectrumComponent.setSampleRate(audioProcessor.getSampleRate());
	m_filterSpectrumComponent.setMagnitudeRangeDb(-24.0f, 24.0f);

	startTimerHz(15.0f);
}

SpectrumMatchFFTAudioProcessorEditor::~SpectrumMatchFFTAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void SpectrumMatchFFTAudioProcessorEditor::timerCallback()
{
	auto& sourceSpectrum = audioProcessor.getSourceSpectrum();
	if (!sourceSpectrum.empty())
	{	
		m_sourceTargetSpectrumCurveComponent.setSpectrum(sourceSpectrum, false);
	}

	auto& targetSpectrum = audioProcessor.getTargetSpectrum();
	if (!targetSpectrum.empty())
	{
		m_sourceTargetSpectrumCurveComponent.setSpectrum(targetSpectrum, true);
	}

	if (!sourceSpectrum.empty() && !targetSpectrum.empty())
	{
		m_filterSpectrumComponent.setSpectrum(audioProcessor.getFilterSpectrum());
	}
}

void SpectrumMatchFFTAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(darkColor);
}

void SpectrumMatchFFTAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	const int pixelSize = width / CANVAS_WIDTH;
	const int pixelSize2 = pixelSize + pixelSize;
	const int pixelSize3 = pixelSize2 + pixelSize;
	const int pixelSize4 = pixelSize3 + pixelSize;
	const int pixelSize5 = pixelSize4 + pixelSize;

	// Set size
	m_pluginLabel.setSize(width, pixelSize2);
	
	m_spectrumGroupLabel.setSize(width, pixelSize);
	m_spectrumDiffGroupLabel.setSize(width, pixelSize);

	m_sourceTargetSpectrumCurveComponent.setSize(width - pixelSize2, pixelSize5);
	m_filterSpectrumComponent.setSize(width - pixelSize2, pixelSize5);

	m_lowPassFilterSlider.setSize(pixelSize3, pixelSize4);
	m_highPassFilterSlider.setSize(pixelSize3, pixelSize4);
	m_frequencyShiftSlider.setSize(pixelSize3, pixelSize4);
	m_resolutionSlider.setSize(pixelSize3, pixelSize4);
	m_ammountSlider.setSize(pixelSize3, pixelSize4);
	m_volumeSlider.setSize(pixelSize3, pixelSize4);

	m_sourceSpectrumButton.setSize(pixelSize3, pixelSize);
	m_targetSpectrumButton.setSize(pixelSize3, pixelSize);

	//Set position
	const int row1 = 0;
	const int row2 = row1 + pixelSize2;
	const int row3 = row2 + pixelSize;
	const int row4 = row3 + pixelSize5;
	const int row5 = row4 + pixelSize;
	const int row6 = row5 + pixelSize5;
	const int row7 = row6 + pixelSize;


	const int column1 = 0;
	const int column2 = column1 + pixelSize;
	const int column3 = column2 + pixelSize3;
	const int column4 = column3 + pixelSize3;
	const int column5 = column4 + pixelSize3;
	const int column6 = column5 + pixelSize3;
	const int column7 = column6 + pixelSize3;
	const int column8 = column7 + pixelSize3;

	m_pluginLabel.setTopLeftPosition(column1, row1);

	m_spectrumGroupLabel.setTopLeftPosition(column1, row2);
	
	m_sourceTargetSpectrumCurveComponent.setTopLeftPosition(column2, row3);

	m_spectrumDiffGroupLabel.setTopLeftPosition(column1, row4);

	m_filterSpectrumComponent.setTopLeftPosition(column2, row5);

	m_sourceSpectrumButton.setTopLeftPosition	(column2, row7 + pixelSize);
	m_targetSpectrumButton.setTopLeftPosition	(column2, row7 + pixelSize2);

	m_highPassFilterSlider.setTopLeftPosition	(column3, row7);
	m_lowPassFilterSlider.setTopLeftPosition	(column4, row7);
	m_frequencyShiftSlider.setTopLeftPosition	(column5, row7);
	m_resolutionSlider.setTopLeftPosition		(column6, row7);
	m_ammountSlider.setTopLeftPosition			(column7, row7);
	m_volumeSlider.setTopLeftPosition			(column8, row7);
}