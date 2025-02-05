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
SpectrumAnalyzerAudioProcessorEditor::SpectrumAnalyzerAudioProcessorEditor (SpectrumAnalyzerAudioProcessor& p)
    : AudioProcessorEditor (&p),
	audioProcessor (p),
	m_spectrumAnalyzer()
{
	addAndMakeVisible(m_spectrumAnalyzer);
	
	setResizable(true, true);

	const int canvasWidth = 18 * 30;
	const int canvasHeight = 8 * 30;

	setSize(canvasWidth, canvasHeight);

	if (auto* constrainer = getConstrainer())
	{
		constexpr int minScale = 50;		// percentage
		constexpr int maxScale = 200;		// percentage

		constrainer->setFixedAspectRatio((double)canvasWidth / (double)canvasHeight);
		constrainer->setSizeLimits(minScale * canvasWidth / 100, minScale * canvasHeight / 100, maxScale * canvasWidth / 100, maxScale * canvasHeight / 100);
	}

	startTimerHz(30);
}

SpectrumAnalyzerAudioProcessorEditor::~SpectrumAnalyzerAudioProcessorEditor()
{
	stopTimer();
}

//==============================================================================
void SpectrumAnalyzerAudioProcessorEditor::timerCallback()
{
	juce::ScopedLock lock(scopeLock);
	
	m_spectrumAnalyzer.setScopeData(audioProcessor.getFrequencySpectrum().getScopeData());
	m_spectrumAnalyzer.repaint();
}

void SpectrumAnalyzerAudioProcessorEditor::paint (juce::Graphics& g)
{
}

void SpectrumAnalyzerAudioProcessorEditor::resized()
{
	const int width = getWidth();
	const int height = getHeight();

	m_spectrumAnalyzer.setSize(width, height);
	m_spectrumAnalyzer.setTopLeftPosition(0, 0);
}
