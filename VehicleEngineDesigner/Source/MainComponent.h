#pragma once

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include <vector>
#include <mutex>
#include <algorithm>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/GUI/WaveformDisplayComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/SpectrogramDisplayComponent.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingOffline.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/RandomNoRepeat.h"
#include "SpectrumMatchRegionProcessor.h"


//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
	//==============================================================================
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	enum SourceType
	{
		Source,
		Output
	};

	enum GenerationType
	{
		NotDefined,
		Flat,
		RandomRegion
	};

	enum InterpolationType
	{
		Point,
		Linear
	};

	enum DisplayMode
	{
		Waveform = 0,
		Spectrogram = 1,
		DominantFrequency = 2
	};

	//==============================================================================
	MainComponent();
	~MainComponent() override;

	static const int CANVAS_WIDTH = 1 + 15 + 1 + 15 + 1 + 15 + 1 + 15 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 11 + 11 + 1 + 1;
	static const int PIXEL_SIZE = 27;
	static const int WRITTE_BIT_DEPTH = 32;		// 32-bit float

	//==============================================================================
	void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint (juce::Graphics& g) override;
	void resized() override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override
	{
	}

	//==========================================================================
	void openSourceButtonClicked()
	{
		zazzDSP::FileIO::openWavFile(
			m_bufferSource,
			m_sampleRate,
			m_formatManager,
			m_sourceFilePath,
			m_wavOpenChooser,
			[this]()
			{
				if (m_bufferSource.getNumSamples() > 0)
				{
					m_fileName = juce::File(m_sourceFilePath).getFileName();

					// Draw waveform
					const float verticalZoom = 1.0f / m_bufferSource.getMagnitude(0, m_bufferSource.getNumSamples());
					m_waveformDisplaySource.setVerticalZoom(verticalZoom);
					m_waveformDisplaySource.setAudioBuffer(m_bufferSource);

					// Draw spectrogram
					m_spectrogramDisplaySource.setAudioBuffer(m_bufferSource);

					// Set filename label
					m_sourceFileNameLabel.setText(m_fileName, juce::dontSendNotification);
				}
			});
	}

	//==========================================================================
	void playSourceButtonClicked()
	{
		if (m_sourceState == TransportState::Stopped)
		{
			if (m_sourceType == SourceType::Source && m_bufferSource.getNumSamples() != 0 || m_sourceType == SourceType::Output && m_bufferOutput.getNumSamples() != 0)
			{
				m_sourceState = TransportState::Playing;
				m_playButton.setButtonText("Stop");
			}
		}
		else if (m_sourceState == TransportState::Playing)
		{
			m_sourceState = TransportState::Stopped;
			m_playButton.setButtonText("Play");
			m_playbackIndex = 0;
		}
	}

	//==========================================================================
	void generateButtonClicked()
	{
		clearRandomSelections();

		generateOutput();
	}

	void generateOutput()
	{
		if (static_cast<GenerationType>(m_generationTypeComboBox.getSelectedId()) == GenerationType::RandomRegion)
		{
			randomRegionsGenerate();
		}
		else
		{
			flatRegionsGenerate();
		}
	}

	//==========================================================================
	void flatRegionsGenerate()
	{
		auto validRegionsCount = getValidRegionsCount();			
		if (validRegionsCount == 0)
		{
			return;
		}

		const auto exportRegionLength = static_cast<int>(m_regionLenghtExportSlider.getValue());
		if (exportRegionLength == 0)
		{
			return;
		}

		const auto sourceSize = m_bufferSource.getNumSamples();
		if (sourceSize == 0)
		{
			return;
		}

		// Initialize spectrum matching if enabled
		if (m_useSpectrumMatching)
		{
			initializeSpectrumMatching();
		}

		// Prepare out buffer
		const auto outputSize = validRegionsCount * exportRegionLength;
		const auto channels = m_bufferSource.getNumChannels();

		{
			std::lock_guard<std::mutex> lock(m_bufferMutex);
			m_bufferOutput.setSize(channels, outputSize);
		}

		for (int channel = 0; channel < channels; channel++)
		{
			auto* pBufferSource = m_bufferSource.getReadPointer(channel);
			auto* pBufferOut = m_bufferOutput.getWritePointer(channel);

			int outIndex = 0;

			for (Region& region : m_regions)
			{
				if (!region.m_isValid)
				{
					continue;
				}

				const int segmentStartIndex = region.m_sampleIndex;
				const int sourceRegionLength = region.m_length;

				// Get resampled region data
				std::vector<float> resampledRegion;
				getResampledRegionData(pBufferSource, segmentStartIndex, sourceRegionLength, exportRegionLength, sourceSize, resampledRegion);

				// Apply spectrum matching if enabled
				if (m_useSpectrumMatching && m_spectrumRegionProcessor)
				{
					std::vector<float> adjustedRegion(exportRegionLength);
					m_spectrumRegionProcessor->applySpectrumAdjustment(resampledRegion.data(), exportRegionLength, adjustedRegion.data());
					std::copy(adjustedRegion.begin(), adjustedRegion.end(), pBufferOut + outIndex);
				}
				else
				{
					std::copy(resampledRegion.begin(), resampledRegion.end(), pBufferOut + outIndex);
				}

				outIndex += exportRegionLength;
			}
		}

		// Set output waveform
		std::vector<Region> regionsExport;

		regionsExport.resize(validRegionsCount);

		int value = 0;
		for (int i = 0; i < validRegionsCount; i++)
		{
			regionsExport[i].m_isValid = true;
			regionsExport[i].m_length = exportRegionLength;
			regionsExport[i].m_sampleIndex = i * exportRegionLength;
		}

		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		m_waveformDisplayOutput.setRegions(regionsExport);
		m_spectrogramDisplayOutput.setAudioBuffer(m_bufferOutput);
	}

	//==========================================================================
	void randomRegionsGenerate()
	{
		const int crossfadeLength = static_cast<int>(m_crossfadeLengthSlider.getValue());
		randomRegionsGenerateWithCrossfade(2 * crossfadeLength);
	}

	//==========================================================================
	void randomRegionsGenerateWithCrossfade(int crossfadeLength)
	{
		const auto exportRegionLeftIdx = (int)m_exportRegionLeftSlider.getValue();
		const auto exportRegionRightIdx = (int)m_exportRegionRightSlider.getValue();
		const auto exportRegionCount = (int)m_exportRegionCountSlider.getValue();

		// Check if setup is valid
		if (exportRegionLeftIdx < 0 ||
			exportRegionRightIdx < 1 ||
			exportRegionLeftIdx >= exportRegionRightIdx ||
			exportRegionCount <= 0)
		{
			return;
		}

		auto tempBufferRegionCount = 0;
		for (int i = exportRegionLeftIdx; i < exportRegionRightIdx; i++)
		{
			tempBufferRegionCount += static_cast<int>(m_regions[i].m_isValid);
		}	

		// Early return if no valid regions exist
		if (tempBufferRegionCount == 0)
		{
			return;
		}

		// Initialize spectrum matching if enabled
		if (m_useSpectrumMatching)
		{
			initializeSpectrumMatching();
		}

		const auto sourceRegionCount = (int)m_regions.size();
		const auto sourceSampleCount = m_bufferSource.getNumSamples();
		const auto exportRegionLength = (int)m_regionLenghtExportSlider.getValue();

		// Check if setup is valid
		if (exportRegionLeftIdx >= sourceRegionCount ||
			exportRegionRightIdx >= sourceRegionCount ||
			exportRegionLeftIdx >= exportRegionRightIdx ||
			exportRegionCount <= 0)
		{
			return;
		}

		const int halfCrossfade = crossfadeLength / 2;

		// Temp audio buffer - allocate extra space for crossfade samples before and after each segment
		juce::AudioBuffer<float> tempBuffer;
		const int tempRegionLength = exportRegionLength + crossfadeLength;
		const int tempBufferLength = tempBufferRegionCount * tempRegionLength;
		const int channels = m_bufferSource.getNumChannels();
		tempBuffer.setSize(channels, tempBufferLength);

		for (int channel = 0; channel < channels; channel++)
		{
			auto* pBufferSource = m_bufferSource.getWritePointer(channel);
			auto* pTempBuffer = tempBuffer.getWritePointer(channel);
			int writeIndex = 0;

			for (int regionIdx = exportRegionLeftIdx; regionIdx < exportRegionRightIdx; regionIdx++)
			{
				// Ignore invalid regions
				if (m_regions[regionIdx].m_isValid == false)
				{
					continue;
				}

				float readIndex = (float)m_regions[regionIdx].m_sampleIndex;
				const int regionLenghtSource = m_regions[regionIdx].m_length;
				const float indexIncrement = (float)regionLenghtSource / (float)exportRegionLength;

				readIndex -= halfCrossfade * indexIncrement;

				// Collect full segment data (fade in + region + fade out)
				std::vector<float> fullSegmentData(tempRegionLength);
				int segmentWriteIndex = 0;

				// Fade in data - with envelope applied BEFORE FFT for proper windowing
				for (int i = 0; i < halfCrossfade; i++)
				{
					float sample = 0.0f;
					if (m_interpolationType == InterpolationType::Point)
					{
						sample = (readIndex >= 0 && readIndex < sourceSampleCount) ? pBufferSource[(int)readIndex] : 0.0f;
					}
					else if (m_interpolationType == InterpolationType::Linear)
					{
						const int indexLeft = (int)readIndex;
						const int indexRight = indexLeft + 1;

						float valueLeft = 0.0f;
						float valueRight = 0.0f;

						if (indexLeft >= 0 && indexLeft < sourceSampleCount)
							valueLeft = pBufferSource[indexLeft];
						if (indexRight >= 0 && indexRight < sourceSampleCount)
							valueRight = pBufferSource[indexRight];

						const float delta = readIndex - std::floor(readIndex);
						sample = valueLeft * (1.0f - delta) + valueRight * delta;
					}

					// Apply fade-in envelope (0 -> 1) BEFORE FFT for loopable segment
					fullSegmentData[segmentWriteIndex] = sample * ((float)(i + 1) / (float)halfCrossfade);
					segmentWriteIndex++;
					readIndex += indexIncrement;
				}

				// Region data (no envelope in middle section)
				for (int i = 0; i < exportRegionLength; i++)
				{
					if (m_interpolationType == InterpolationType::Point)
					{
						fullSegmentData[segmentWriteIndex] = (readIndex >= 0 && readIndex < sourceSampleCount) ? pBufferSource[(int)readIndex] : 0.0f;
					}
					else if (m_interpolationType == InterpolationType::Linear)
					{
						const int indexLeft = (int)readIndex;
						const int indexRight = indexLeft + 1;

						float valueLeft = 0.0f;
						float valueRight = 0.0f;

						if (indexLeft >= 0 && indexLeft < sourceSampleCount)
							valueLeft = pBufferSource[indexLeft];
						if (indexRight >= 0 && indexRight < sourceSampleCount)
							valueRight = pBufferSource[indexRight];

						const float delta = readIndex - std::floor(readIndex);
						const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

						fullSegmentData[segmentWriteIndex] = interpolated;
					}

					segmentWriteIndex++;
					readIndex += indexIncrement;
				}

				// Fade out data - with envelope applied BEFORE FFT for proper windowing
				for (int i = 0; i < halfCrossfade; i++)
				{
					float sample = 0.0f;
					if (m_interpolationType == InterpolationType::Point)
					{
						sample = (readIndex >= 0 && readIndex < sourceSampleCount) ? pBufferSource[(int)readIndex] : 0.0f;
					}
					else if (m_interpolationType == InterpolationType::Linear)
					{
						const int indexLeft = (int)readIndex;
						const int indexRight = indexLeft + 1;

						float valueLeft = 0.0f;
						float valueRight = 0.0f;

						if (indexLeft >= 0 && indexLeft < sourceSampleCount)
							valueLeft = pBufferSource[indexLeft];
						if (indexRight >= 0 && indexRight < sourceSampleCount)
							valueRight = pBufferSource[indexRight];

						const float delta = readIndex - std::floor(readIndex);
						sample = valueLeft * (1.0f - delta) + valueRight * delta;
					}

					// Apply fade-out envelope (1 -> 0) BEFORE FFT for loopable segment
					fullSegmentData[segmentWriteIndex] = sample * (1.0f - ((float)(i + 1) / (float)halfCrossfade));
					segmentWriteIndex++;
					readIndex += indexIncrement;
				}

				// Apply spectrum matching to entire segment if enabled
				std::vector<float> processedSegment(tempRegionLength);
				if (m_useSpectrumMatching && m_spectrumRegionProcessor)
				{
					m_spectrumRegionProcessor->applySpectrumAdjustment(fullSegmentData.data(), tempRegionLength, processedSegment.data());

					// Apply fade envelopes AGAIN after spectrum matching to ensure smooth boundaries post-FFT
					// Apply fade-in envelope to first halfCrossfade samples
					for (int i = 0; i < halfCrossfade; i++)
					{
						processedSegment[i] *= ((float)(i + 1) / (float)halfCrossfade);
					}

					// Apply fade-out envelope to last halfCrossfade samples
					for (int i = 0; i < halfCrossfade; i++)
					{
						processedSegment[halfCrossfade + exportRegionLength + i] *= (1.0f - ((float)(i + 1) / (float)halfCrossfade));
					}
				}
				else
				{
					std::copy(fullSegmentData.begin(), fullSegmentData.end(), processedSegment.begin());
				}

				std::copy(processedSegment.begin(), processedSegment.end(), pTempBuffer + writeIndex);
				writeIndex += tempRegionLength;
			}
		}

		// Prepare out buffer with overlapping regions
		{
			std::lock_guard<std::mutex> lock(m_bufferMutex);

			// Use saved random selections if available, otherwise generate new ones
			if (m_randomRegionSelections.empty() || (int)m_randomRegionSelections.size() != exportRegionCount)
			{
				// Generate new random selections
				m_randomRegionSelections.clear();
				RandomNoRepeat randomNoRepeat(0, tempBufferRegionCount - 1, tempBufferRegionCount / 2);

				for (int outRegionIdx = 0; outRegionIdx < exportRegionCount; outRegionIdx++)
				{
					m_randomRegionSelections.push_back(randomNoRepeat.get());
				}
			}

			// Calculate output size accounting for crossfade overlaps
			const int outputSize = exportRegionCount * exportRegionLength;
			m_bufferOutput.setSize(channels, outputSize);
			m_bufferOutput.clear();

			for (int outRegionIdx = 0; outRegionIdx < exportRegionCount; outRegionIdx++)
			{
				if (outRegionIdx >= (int)m_randomRegionSelections.size())
				{
					break;  // Safety check to prevent out-of-bounds access
				}
				const int regionIndex = m_randomRegionSelections[outRegionIdx];
				
				for (int channel = 0; channel < channels; channel++)
				{
					auto* pTempBuffer = tempBuffer.getReadPointer(channel);
					auto* pOutBuffer = m_bufferOutput.getWritePointer(channel);

					int readIndex = regionIndex * tempRegionLength;
					int writeIndex = outRegionIdx * exportRegionLength - halfCrossfade;

					for (int sample = 0; sample < tempRegionLength; sample++)
					{
						if (writeIndex >= 0 && writeIndex < outputSize)
						{
							pOutBuffer[writeIndex] = pOutBuffer[writeIndex] + pTempBuffer[readIndex];
						}
							
						readIndex++;
						writeIndex++;
					}
				}
			}
		}

		// Set output waveform display
		std::vector<Region> regionsExport;
		regionsExport.resize(exportRegionCount);

		int currentPos = 0;
		for (int i = 0; i < exportRegionCount; i++)
		{
			regionsExport[i].m_sampleIndex = currentPos;
			regionsExport[i].m_isValid = true;

			// Calculate region length accounting for crossfades
			if (i < exportRegionCount - 1)
			{
				regionsExport[i].m_length = exportRegionLength;
				currentPos += exportRegionLength;  // Full length for position calculation
			}
			else
			{
				regionsExport[i].m_length = m_bufferOutput.getNumSamples() - currentPos;
			}
		}

		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		m_waveformDisplayOutput.setRegions(regionsExport);
		m_spectrogramDisplayOutput.setAudioBuffer(m_bufferOutput);
	}

	//==========================================================================
	void saveWavButtonClicked()
	{
		if (const int samples = m_bufferOutput.getNumSamples(); samples != 0)
		{
			zazzDSP::FileIO::saveWavFile(m_bufferOutput, m_sampleRate, m_wavSaveChooser, WRITTE_BIT_DEPTH);
		}
		else
		{
			juce::AlertWindow::showMessageBoxAsync(
				juce::AlertWindow::WarningIcon,
				"Error",
				"Output is empty!");
		}
	}

	//==========================================================================
	void getResampledRegionData(const float* pBufferChan, int segmentStartIndex, int sourceRegionLength, 
								int exportRegionLength, const int sourceSize, std::vector<float>& outData)
	{
		outData.resize(exportRegionLength);
		const float indexIncrement = (float)sourceRegionLength / (float)exportRegionLength;
		float sourceIndex = 0.0f;

		for (int i = 0; i < exportRegionLength; i++)
		{
			if (m_interpolationType == InterpolationType::Point)
			{
				outData[i] = pBufferChan[segmentStartIndex + (int)sourceIndex];
			}
			else if (m_interpolationType == InterpolationType::Linear)
			{
				const int indexLeft = segmentStartIndex + (int)sourceIndex;
				const int indexRight = indexLeft < sourceSize - 1 ? indexLeft + 1 : indexLeft;

				const float valueLeft = pBufferChan[indexLeft];
				const float valueRight = pBufferChan[indexRight];

				const float delta = sourceIndex - std::floor(sourceIndex);
				const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

				outData[i] = interpolated;
			}

			sourceIndex += indexIncrement;
		}
	}

	//==========================================================================
	void exportRegionsButtonClicked()
	{
		// Check if there are valid regions to export
		int validRegionsCount = 0;
		for (const auto& region : m_regions)
		{
			if (region.m_isValid)
				validRegionsCount++;
		}

		if (validRegionsCount == 0)
		{
			juce::AlertWindow::showMessageBoxAsync(
				juce::AlertWindow::WarningIcon,
				"Error",
				"No valid regions to export!");
			return;
		}

		if (m_bufferSource.getNumSamples() == 0)
		{
			juce::AlertWindow::showMessageBoxAsync(
				juce::AlertWindow::WarningIcon,
				"Error",
				"Source buffer is empty!");
			return;
		}

		// Get export region length
		const int exportRegionLength = static_cast<int>(m_regionLenghtExportSlider.getValue());
		if (exportRegionLength == 0)
		{
			juce::AlertWindow::showMessageBoxAsync(
				juce::AlertWindow::WarningIcon,
				"Error",
				"Export region length must be greater than 0!");
			return;
		}

		// Open directory chooser
		if (!m_regionsExportChooser)
		{
			m_regionsExportChooser = std::make_unique<juce::FileChooser>(
				"Select directory to export regions",
				juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
				"");
		}

		auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

		m_regionsExportChooser->launchAsync(flags, [this, exportRegionLength](const juce::FileChooser& fc)
		{
			juce::File selectedDirectory = fc.getResult();
			if (selectedDirectory == juce::File())
				return;

			if (!selectedDirectory.isDirectory())
				return;

			// Extract base filename from source file (without extension)
			juce::String baseFileName = juce::File(m_sourceFilePath).getFileNameWithoutExtension();

			// Initialize spectrum matching if enabled
			if (m_useSpectrumMatching)
			{
				initializeSpectrumMatching();
			}

			// Calculate padding width based on total number of regions
			// Use the index of the last region to determine padding
			// 1-9 regions: no padding (0, 1, ..., 9)
			// 10-99 regions: 2 digits (00, 01, ..., 99)
			// 100-999 regions: 3 digits (000, 001, ..., 999)
			// etc.
			int paddingWidth = juce::String((int)m_regions.size() - 1).length();

			const auto channels = m_bufferSource.getNumChannels();
			auto* pBufferSource = m_bufferSource.getWritePointer(0);
			const int sourceSize = m_bufferSource.getNumSamples();

			int exportedCount = 0;

			// Export each valid region
			for (int regionIdx = 0; regionIdx < (int)m_regions.size(); ++regionIdx)
			{
				const auto& region = m_regions[regionIdx];

				if (!region.m_isValid)
					continue;

				// Create buffer for this region
				juce::AudioBuffer<float> regionBuffer(channels, exportRegionLength);

				const int segmentStartIndex = region.m_sampleIndex;
				const int sourceRegionLength = region.m_length;
				const float indexIncrement = (float)sourceRegionLength / (float)exportRegionLength;

				// Resample and interpolate region data
				for (int channel = 0; channel < channels; channel++)
				{
					auto* pBufferChan = m_bufferSource.getReadPointer(channel);
					auto* pRegionBuffer = regionBuffer.getWritePointer(channel);

					// Get resampled region
					std::vector<float> resampledRegion;
					getResampledRegionData(pBufferChan, segmentStartIndex, sourceRegionLength, exportRegionLength, sourceSize, resampledRegion);

					// Apply spectrum matching if enabled
					if (m_useSpectrumMatching && m_spectrumRegionProcessor)
					{
						m_spectrumRegionProcessor->applySpectrumAdjustment(resampledRegion.data(), exportRegionLength, pRegionBuffer);
					}
					else
					{
						std::copy(resampledRegion.begin(), resampledRegion.end(), pRegionBuffer);
					}
				}

				// Create filename with dynamic padding using region's actual ID: <basename>_<padded_region_id>.wav
				juce::String paddedRegionId = juce::String(regionIdx);
				while (paddedRegionId.length() < paddingWidth)
					paddedRegionId = "0" + paddedRegionId;
				juce::String filename = baseFileName + "_" + paddedRegionId + ".wav";
				juce::File outputFile = selectedDirectory.getChildFile(filename);

				// Save region as WAV file
				if (outputFile.create() || outputFile.exists())
				{
					if (auto stream = outputFile.createOutputStream())
					{
						juce::WavAudioFormat wavFormat;
						if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(
							wavFormat.createWriterFor(stream.get(), m_sampleRate, regionBuffer.getNumChannels(), 16, {}, 0)))
						{
							writer->writeFromAudioSampleBuffer(regionBuffer, 0, regionBuffer.getNumSamples());
							writer.reset();
							stream.release();
						}
					}
				}

				exportedCount++;
			}

			// Show confirmation
			juce::AlertWindow::showMessageBoxAsync(
				juce::AlertWindow::InfoIcon,
				"Success",
				juce::String(exportedCount) + " regions exported successfully!");
		});
	}

	//==========================================================================
	void newProjectButtonClicked()
	{
		// Clear all buffers
		{
			std::lock_guard<std::mutex> lock(m_bufferMutex);
			m_bufferSource.setSize(0, 0);
			m_bufferOutput.setSize(0, 0);
		}
		m_regions.clear();
		m_randomRegionSelections.clear();

		// Clear file information
		m_fileName = "";
		m_sourceFilePath = "";
		m_sourceFileNameLabel.setText("", juce::dontSendNotification);

		// Reset playback state
		m_playbackIndex = 0;
		m_sourceState = TransportState::Stopped;
		m_playButton.setButtonText("Play");

		// Reset source type
		m_sourceType = SourceType::Source;
		m_sourceButton.setButtonText("Source");

		// Reset interpolation type
		m_interpolationType = InterpolationType::Linear;

		// Reset region median
		m_regionLenghtMedian = 0;

		// Reset all sliders to default values
		m_thresholdSlider.setValue(-60.0);
		m_minimumLengthSlider.setValue(100.0);
		m_SpectrumDifferenceSlider.setValue(100.0);
		m_zeroCrossingCountSlider.setValue(1.0);
		m_crossfadeLengthSlider.setValue(100.0);
		m_regionLenghtExportSlider.setValue(1000.0);
		m_exportRegionLeftSlider.setValue(0.0);
		m_exportRegionRightSlider.setValue(0.0);
		m_exportMaxRegionOffsetSlider.setValue(1000.0);
		m_exportRegionCountSlider.setValue(200.0);

		// Reset combo boxes to default
		m_detectionTypeComboBox.setSelectedId(1);
		m_generationTypeComboBox.setSelectedId(1);

		// Clear all labels
		m_regionsCountLabel.setText("", juce::dontSendNotification);
		m_validRegionsCountLabel.setText("", juce::dontSendNotification);
		m_regionLenghtMedianLabel.setText("", juce::dontSendNotification);
		m_regionLengthDiffLabel.setText("", juce::dontSendNotification);
		m_maxZeroCrossingGainLabel.setText("", juce::dontSendNotification);

		// Clear waveform displays
		m_waveformDisplaySource.setAudioBuffer(m_bufferSource);
		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);

		// Repaint to update UI
		repaint();
	}

	//==========================================================================
	void saveProjectButtonClicked()
	{
		m_projectSaveChooser = std::make_unique<juce::FileChooser>(
			"Save Project File",
			juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
			"*.json");

		auto flags = juce::FileBrowserComponent::saveMode;

		m_projectSaveChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
		{
			juce::File projectFile = fc.getResult();
			if (projectFile == juce::File())
				return;

			auto projectObject = std::make_unique<juce::DynamicObject>();

			// Save file path
			projectObject->setProperty("sourceFilePath", m_sourceFilePath);

			// Save all slider values
			projectObject->setProperty("threshold", m_thresholdSlider.getValue());
			projectObject->setProperty("minimumLength", m_minimumLengthSlider.getValue());
			projectObject->setProperty("spectrumDifference", m_SpectrumDifferenceSlider.getValue());
			projectObject->setProperty("fftPhaseThreshold", m_fftPhaseThresholdSlider.getValue());
			projectObject->setProperty("zeroCrossingCount", m_zeroCrossingCountSlider.getValue());
			projectObject->setProperty("crossfadeLength", m_crossfadeLengthSlider.getValue());
			projectObject->setProperty("regionLenghtExport", m_regionLenghtExportSlider.getValue());
			projectObject->setProperty("exportRegionLeft", m_exportRegionLeftSlider.getValue());
			projectObject->setProperty("exportRegionRight", m_exportRegionRightSlider.getValue());
			projectObject->setProperty("exportMaxRegionOffset", m_exportMaxRegionOffsetSlider.getValue());
			projectObject->setProperty("exportRegionCount", m_exportRegionCountSlider.getValue());

			// Save combo box selections
			projectObject->setProperty("detectionType", m_detectionTypeComboBox.getSelectedId());
			projectObject->setProperty("generationType", m_generationTypeComboBox.getSelectedId());

			// Save interpolation type
			projectObject->setProperty("interpolationType", (int)m_interpolationType);

			// Save detection results
			projectObject->setProperty("regionLenghtMedian", m_regionLenghtMedian);
			projectObject->setProperty("regions", serializeRegions());

			// Save generation results
			projectObject->setProperty("randomRegionSelections", serializeRandomSelections());

			juce::var jsonVar(projectObject.release());
			projectFile.replaceWithText(juce::JSON::toString(jsonVar, true));
		});
	}

	//==========================================================================
	void loadProjectButtonClicked()
	{
		m_projectLoadChooser = std::make_unique<juce::FileChooser>(
			"Load Project File",
			juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
			"*.json");

		auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

		m_projectLoadChooser->launchAsync(flags, [this](const juce::FileChooser& fc)
		{
			juce::File projectFile = fc.getResult();
			if (projectFile == juce::File())
				return;

			juce::String fileContents = projectFile.loadFileAsString();
			auto jsonVar = juce::JSON::parse(fileContents);

			if (jsonVar.isObject())
			{
				auto* obj = jsonVar.getDynamicObject();

				// Load source file if path exists
				if (obj->hasProperty("sourceFilePath"))
				{
					juce::String filePath = obj->getProperty("sourceFilePath").toString();
					juce::File sourceFile(filePath);

					if (sourceFile.exists())
					{
						auto* reader = m_formatManager.createReaderFor(sourceFile);
						if (reader != nullptr)
						{
							const int samples = (int)reader->lengthInSamples;
							if (samples != 0)
							{
								m_sampleRate = static_cast<int>(reader->sampleRate);
								m_fileName = sourceFile.getFileName();
								m_sourceFilePath = sourceFile.getFullPathName();
								m_bufferSource.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
								reader->read(&m_bufferSource, 0, (int)reader->lengthInSamples, 0, true, true);

									// Draw waveform
									const float verticalZoom = 1.0f / m_bufferSource.getMagnitude(0, m_bufferSource.getNumSamples());
									m_waveformDisplaySource.setVerticalZoom(verticalZoom);
									m_waveformDisplaySource.setAudioBuffer(m_bufferSource);

									// Draw spectrogram
									m_spectrogramDisplaySource.setAudioBuffer(m_bufferSource);

									// Set filename label
									m_sourceFileNameLabel.setText(m_fileName, juce::dontSendNotification);
								}

								delete reader;
						}
					}
				}

				// Load slider values if they exist
				if (obj->hasProperty("threshold"))
					m_thresholdSlider.setValue(obj->getProperty("threshold"));
				if (obj->hasProperty("minimumLength"))
					m_minimumLengthSlider.setValue(obj->getProperty("minimumLength"));
				if (obj->hasProperty("spectrumDifference"))
					m_SpectrumDifferenceSlider.setValue(obj->getProperty("spectrumDifference"));
				if (obj->hasProperty("fftPhaseThreshold"))
					m_fftPhaseThresholdSlider.setValue(obj->getProperty("fftPhaseThreshold"));
				if (obj->hasProperty("zeroCrossingCount"))
					m_zeroCrossingCountSlider.setValue(obj->getProperty("zeroCrossingCount"));
				if (obj->hasProperty("crossfadeLength"))
					m_crossfadeLengthSlider.setValue(obj->getProperty("crossfadeLength"));
				if (obj->hasProperty("regionLenghtExport"))
					m_regionLenghtExportSlider.setValue(obj->getProperty("regionLenghtExport"));
				if (obj->hasProperty("exportRegionLeft"))
					m_exportRegionLeftSlider.setValue(obj->getProperty("exportRegionLeft"));
				if (obj->hasProperty("exportRegionRight"))
					m_exportRegionRightSlider.setValue(obj->getProperty("exportRegionRight"));
				if (obj->hasProperty("exportMaxRegionOffset"))
					m_exportMaxRegionOffsetSlider.setValue(obj->getProperty("exportMaxRegionOffset"));
				if (obj->hasProperty("exportRegionCount"))
					m_exportRegionCountSlider.setValue(obj->getProperty("exportRegionCount"));

				// Load combo box selections if they exist
				if (obj->hasProperty("detectionType"))
					m_detectionTypeComboBox.setSelectedId(obj->getProperty("detectionType"));
				if (obj->hasProperty("generationType"))
					m_generationTypeComboBox.setSelectedId(obj->getProperty("generationType"));

				// Load interpolation type if it exists
				if (obj->hasProperty("interpolationType"))
					m_interpolationType = static_cast<InterpolationType>((int)obj->getProperty("interpolationType"));

				// Load detection results if they exist
				if (obj->hasProperty("regionLenghtMedian"))
					m_regionLenghtMedian = (int)obj->getProperty("regionLenghtMedian");

				if (obj->hasProperty("regions"))
					deserializeRegions(obj->getProperty("regions"));

				// Load generation results if they exist
				if (obj->hasProperty("randomRegionSelections"))
					deserializeRandomSelections(obj->getProperty("randomRegionSelections"));

				// Update UI labels and displays
				if (m_bufferSource.getNumSamples() > 0 && !m_regions.empty())
				{
					m_regionsCountLabel.setText("Regions count: " + juce::String((float)m_regions.size(), 0), juce::dontSendNotification);
					m_regionLenghtMedianLabel.setText("Lenght median: " + juce::String((float)m_regionLenghtMedian, 0), juce::dontSendNotification);

					// Get max median diff
					int maxDiff = 0;
					for (int i = 0; i < m_regions.size(); i++)
					{
						const int medianDiff = std::abs(m_regionLenghtMedian - m_regions[i].m_length);
						if (medianDiff > maxDiff)
						{
							maxDiff = medianDiff;
						}
					}
					m_regionLengthDiffLabel.setText("Max median diff: " + juce::String((float)maxDiff, 0), juce::dontSendNotification);

					// Handle max zero crossing
					float maxZeroCrossing = 0.0f;
					auto* channelData = m_bufferSource.getReadPointer(0);
					for (int i = 0; i < m_regions.size(); i++)
					{
						const float value = channelData[m_regions[i].m_sampleIndex];
						maxZeroCrossing = std::fmaxf(maxZeroCrossing, value);
					}
					m_maxZeroCrossingGainLabel.setText("Max. zero crossing: " + juce::String((float)juce::Decibels::gainToDecibels(maxZeroCrossing), 1), juce::dontSendNotification);

					// Update valid regions count
					auto validRegionsCount = getValidRegionsCount();
					m_validRegionsCountLabel.setText("Valid regions count: " + juce::String((float)validRegionsCount, 0), juce::dontSendNotification);

					// Update waveform display
					m_waveformDisplaySource.setRegions(m_regions);

					// Update slider ranges
					if (const size_t size = m_regions.size(); size > 1)
					{
						m_exportRegionLeftSlider.setRange(0.0, (double)(size - 1), 1.0);
						m_exportRegionRightSlider.setRange(0.0, (double)(size - 1), 1.0);
						m_exportRegionLeftSlider.setValue(0.0);
						m_exportRegionRightSlider.setValue((double)(size - 1));
					}
					else
					{
						m_exportRegionLeftSlider.setRange(0.0, 1.0, 1.0);
						m_exportRegionRightSlider.setRange(0.0, 1.0, 1.0);
						m_exportRegionLeftSlider.setValue(0.0);
						m_exportRegionRightSlider.setValue(0.0);
					}

					// Run generate with loaded parameters to create output buffer
					generateOutput();

					repaint();
				}
			}
		});
	}

	//==========================================================================
	void updateValidZeroCrossingIdx()
	{
		if (m_regions.empty())
		{
			return;
		}

		const int regionOffset = (int)m_exportMaxRegionOffsetSlider.getValue();
		const int leftThreshold = m_regionLenghtMedian - regionOffset;
		const int rightThreshold = m_regionLenghtMedian + regionOffset;

		// Limit regions by offset
		for (auto& region : m_regions)
		{
			region.m_isValid = region.m_length > leftThreshold && region.m_length < rightThreshold;
		}

		// Compare spectrum
		const int FFT_ORDER = 10;
		const int FFT_SIZE = 1 << FFT_ORDER;

		juce::dsp::FFT forwardFFT(FFT_ORDER);
		juce::dsp::WindowingFunction<float> window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann);
		float tempBuffer[FFT_SIZE];
		float fftData[2 * FFT_SIZE];
		auto* pBufferSource = m_bufferSource.getWritePointer(0);
		const int sourceSize = m_bufferSource.getNumSamples();


		for (auto& region : m_regions)
		{
			if (region.m_isValid == false)
			{
				continue;
			}

			// Resample
			const int segmentStartIndex = region.m_sampleIndex;
			const int sourceRegionLength = region.m_length;
			const float indexIncrement = (float)sourceRegionLength / (float)FFT_SIZE;

			float sourceIndex = 0.0f;

			for (int i = 0; i < FFT_SIZE; i++)
			{
				const int indexLeft = segmentStartIndex + (int)sourceIndex;
				const int indexRight = indexLeft < sourceSize - 1 ? indexLeft + 1 : indexLeft;

				const float valueLeft = pBufferSource[indexLeft];
				const float valueRight = pBufferSource[indexRight];

				const float delta = sourceIndex - std::floor(sourceIndex);
				const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

				//tempBuffer[i] = interpolated;
				region.m_fftData[i] = interpolated;

				sourceIndex += indexIncrement;
			}

			// Get spectrum
			// Apply windowing function
			window.multiplyWithWindowingTable(region.m_fftData, FFT_SIZE);

			// Perform FFT
			forwardFFT.performFrequencyOnlyForwardTransform(region.m_fftData);
		}

		// Do FFT statistics		
		auto validRegionsCount = getValidRegionsCount();

		// Get average
		float fftDataAverage[2 * FFT_SIZE];
		std::fill(std::begin(fftDataAverage), std::end(fftDataAverage), 0.0f);

		for (auto& region : m_regions)
		{
			if (region.m_isValid == false)
			{
				continue;
			}

			for (size_t i = 0; i < 2 * FFT_SIZE; i++)
			{
				const float value = region.m_fftData[i];
				if (value > 0.001f)
				{
					fftDataAverage[i] += value;
				}
			}
		}

		for (size_t i = 0; i < 2 * FFT_SIZE; i++)
		{
			fftDataAverage[i] = fftDataAverage[i] / (float)(validRegionsCount);
		}

		// Get difference
		const auto spectrumDifference = 0.01 * m_SpectrumDifferenceSlider.getValue();

		for (auto& region : m_regions)
		{
			if (region.m_isValid == false)
			{
				continue;
			}

			float diff = 0.0f;

			for (size_t i = 0; i < FFT_SIZE; i++)
			{
				const float d = std::fabsf(fftDataAverage[i] - region.m_fftData[i]);
				if (d > 0.001f)
				{
					diff += d;
				}
			}

			// Set invalid
			region.m_difference = diff / (float)(FFT_SIZE);
			if (region.m_difference > spectrumDifference)
			{
				region.m_isValid = false;
			}
		}


		validRegionsCount = getValidRegionsCount();
		m_validRegionsCountLabel.setText("Valid regions count: " + juce::String((float)validRegionsCount, 0), juce::dontSendNotification);
		m_waveformDisplaySource.setRegions(m_regions);

		// Update spectrum source combo box with valid regions
		m_spectrumSourceComboBox.clear(juce::dontSendNotification);
		m_spectrumSourceComboBox.addItem("Average", 1);
		int validRegionIdx = 0;
		for (size_t i = 0; i < m_regions.size(); ++i)
		{
			if (m_regions[i].m_isValid)
			{
				m_spectrumSourceComboBox.addItem("Region " + juce::String(static_cast<int>(i)), 2 + validRegionIdx);
				validRegionIdx++;
			}
		}
		m_spectrumSourceComboBox.setSelectedId(1, juce::dontSendNotification); // Default to "Average"
	}

	//==========================================================================
	void spectrumSourceComboBoxChanged()
	{
		int selectedId = m_spectrumSourceComboBox.getSelectedId();

		if (selectedId == 1)
		{
			// "Average" mode
			m_selectedSpectrumRegionIndex = -1;
		}
		else
		{
			// Specific region selected (selectedId - 2 because ID 1 is "Average")
			m_selectedSpectrumRegionIndex = selectedId - 2;
		}

		// Note: Spectrum matching will be initialized only when Generate or Export buttons are pressed
	}

	//==========================================================================
	void spectrumMatchToggleClicked()
	{
		m_useSpectrumMatching = m_spectrumMatchToggle.getToggleState();

		// If enabling and we have valid regions, initialize spectrum processor
		if (m_useSpectrumMatching && getValidRegionsCount() > 0)
		{
			initializeSpectrumMatching();
		}
	}

	//==========================================================================
	void initializeSpectrumMatching()
	{
		if (getValidRegionsCount() == 0 || m_bufferSource.getNumSamples() == 0)
		{
			return;
		}

		// Create processor if not already created
		if (!m_spectrumRegionProcessor)
		{
			static constexpr int FFT_ORDER = 14;
			static constexpr int FFT_SIZE = 1 << FFT_ORDER;
			m_spectrumRegionProcessor = std::make_unique<SpectrumMatchRegionProcessor>(FFT_SIZE);
		}

		auto* pBufferSource = m_bufferSource.getReadPointer(0);

		// Check if using average or specific region
		if (m_selectedSpectrumRegionIndex < 0)
		{
			// Use average spectrum from all valid regions
			std::vector<int> validRegionSizes;
			std::vector<int> validRegionPositions;

			for (const auto& region : m_regions)
			{
				if (region.m_isValid)
				{
					validRegionSizes.push_back(region.m_length);
					validRegionPositions.push_back(region.m_sampleIndex);
				}
			}

			// Calculate average spectrum from valid regions
			m_spectrumRegionProcessor->calculateAverageSpectrum(validRegionSizes, pBufferSource, validRegionPositions);
		}
		else
		{
			// Use spectrum from selected region
			if (m_selectedSpectrumRegionIndex < static_cast<int>(m_regions.size()))
			{
				const auto& selectedRegion = m_regions[m_selectedSpectrumRegionIndex];
				if (selectedRegion.m_isValid && selectedRegion.m_length > 0)
				{
					m_spectrumRegionProcessor->calculateRegionSpectrum(
						pBufferSource + selectedRegion.m_sampleIndex,
						selectedRegion.m_length
					);
				}
			}
		}
	}

	//==========================================================================
	int getValidRegionsCount()
	{
		auto validRegionsCount = 0;
		for (Region& region : m_regions)
		{
			validRegionsCount += static_cast<int>(region.m_isValid);
		}

		return validRegionsCount;
	}
	
	//==========================================================================
	void detectRegionsButtonClicked()
	{		
		if (m_bufferSource.getNumSamples() == 0)
		{
			return;
		}

		ZeroCrossingOffline zeroCrossing{};
		zeroCrossing.init(m_sampleRate);

		const float maximumFrequencySamples = (float)m_minimumLengthSlider.getValue();
		const float maximumFrequencyHz = m_sampleRate / maximumFrequencySamples;

		zeroCrossing.set((float)juce::Decibels::decibelsToGain(m_thresholdSlider.getValue()), maximumFrequencyHz);
		zeroCrossing.setType(m_detectionTypeComboBox.getSelectedId());
		zeroCrossing.setFFTPhaseThreshold((float)m_fftPhaseThresholdSlider.getValue());

		// Check if using Filter detection type to prepare filtered buffer
		const bool isFilterType = (m_detectionTypeComboBox.getSelectedId() == 2); // 2 = "Filter"
		juce::AudioBuffer<float> filteredBuffer;
		juce::AudioBuffer<float> bufferForProcessing;

		if (isFilterType && m_bufferSource.getNumSamples() > 0)
		{
			// Create filtered copy with zero-phase filtering
			const float highPassFrequency = maximumFrequencyHz * 0.7f;
			const float lowPassFrequency = maximumFrequencyHz * 1.3f;

			// Forward filtering pass
			juce::AudioBuffer<float> tempBuffer;
			tempBuffer.makeCopyOf(m_bufferSource, true);

			BiquadFilter filterHP, filterLP;
			filterHP.init(m_sampleRate);
			filterLP.init(m_sampleRate);
			filterHP.setHighPass(highPassFrequency, 2.0f);
			filterLP.setLowPass(lowPassFrequency, 2.0f);

			// Forward pass
			for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
			{
				auto* data = tempBuffer.getWritePointer(channel);
				for (int i = 0; i < tempBuffer.getNumSamples(); ++i)
				{
					float sample = data[i];
					sample = filterHP.processDF1(sample);
					sample = filterLP.processDF1(sample);
					data[i] = sample;
				}
				filterHP.reset();
				filterLP.reset();
			}

			// Reverse for backward pass
			for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
			{
				auto* data = tempBuffer.getWritePointer(channel);
				std::reverse(data, data + tempBuffer.getNumSamples());
			}

			// Backward pass
			for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
			{
				auto* data = tempBuffer.getWritePointer(channel);
				for (int i = 0; i < tempBuffer.getNumSamples(); ++i)
				{
					float sample = data[i];
					sample = filterHP.processDF1(sample);
					sample = filterLP.processDF1(sample);
					data[i] = sample;
				}
				filterHP.reset();
				filterLP.reset();
			}

			// Reverse back to original order
			for (int channel = 0; channel < tempBuffer.getNumChannels(); ++channel)
			{
				auto* data = tempBuffer.getWritePointer(channel);
				std::reverse(data, data + tempBuffer.getNumSamples());
			}

			filteredBuffer.makeCopyOf(tempBuffer, true);
			bufferForProcessing.makeCopyOf(filteredBuffer, true);
		}
		else
		{
			// Use original buffer for processing
			bufferForProcessing.makeCopyOf(m_bufferSource, true);
		}

		std::vector<int> zeroCrossingIdxs;
		zeroCrossing.process(bufferForProcessing, zeroCrossingIdxs);

		const size_t size = zeroCrossingIdxs.size();

		// Get zero crossing grouping value
		const int zcGroupCount = static_cast<int>(m_zeroCrossingCountSlider.getValue());

		// Create regions
		m_regions.clear();

		if (size != 0)
		{
			if (zcGroupCount == 1)
			{
				// Original behavior: each zero crossing interval is a region
				m_regions.resize(zeroCrossingIdxs.size());

				for (int i = 0; i < zeroCrossingIdxs.size() - 1; i++)
				{
					m_regions[i].m_sampleIndex = zeroCrossingIdxs[i];
					m_regions[i].m_length = zeroCrossingIdxs[i + 1] - zeroCrossingIdxs[i];
					m_regions[i].m_isValid = true;
				}

				// Handle last region
				m_regions[zeroCrossingIdxs.size() - 1].m_isValid = true;
				m_regions[zeroCrossingIdxs.size() - 1].m_sampleIndex = zeroCrossingIdxs[zeroCrossingIdxs.size() - 1];
				m_regions[zeroCrossingIdxs.size() - 1].m_length = m_bufferSource.getNumSamples() - zeroCrossingIdxs[zeroCrossingIdxs.size() - 1];
			}
			else
			{
				// Grouped behavior: group N consecutive zero crossings into single region
				std::vector<Region> groupedRegions;

				for (int i = 0; i < zeroCrossingIdxs.size(); i += zcGroupCount)
				{
					Region region;
					region.m_sampleIndex = zeroCrossingIdxs[i];
					region.m_isValid = true;

					// Determine end of this group
					int endIdx = i + zcGroupCount;
					if (endIdx >= zeroCrossingIdxs.size())
					{
						endIdx = zeroCrossingIdxs.size() - 1;
					}

					// Length from start of group to end of group
					if (endIdx < zeroCrossingIdxs.size() - 1)
					{
						region.m_length = zeroCrossingIdxs[endIdx] - zeroCrossingIdxs[i];
					}
					else
					{
						// Last group extends to end of buffer
						region.m_length = m_bufferSource.getNumSamples() - zeroCrossingIdxs[i];
					}

					groupedRegions.push_back(region);
				}

				m_regions = groupedRegions;
			}
		}

		m_regionsCountLabel.setText("Regions count: " + juce::String((float)m_regions.size(), 0), juce::dontSendNotification);

		// Get median
		m_regionLenghtMedian = getMedian(m_regions);
		m_regionLenghtMedianLabel.setText("Lenght median: " + juce::String((float)m_regionLenghtMedian, 0), juce::dontSendNotification);
		m_regionLenghtExportSlider.setValue(m_regionLenghtMedian);

		// Get max median diff
		int maxDiff = 0;
		for (int i = 0; i < m_regions.size(); i++)
		{
			const int medianDiff = std::abs(m_regionLenghtMedian - m_regions[i].m_length);

			if (medianDiff > maxDiff)
			{
				maxDiff = medianDiff;
			}
		}

		m_regionLengthDiffLabel.setText("Max median diff: " + juce::String((float)maxDiff, 0), juce::dontSendNotification);		

		// Handle max zero crossing
		float maxZeroCrossing = 0.0f;
		auto* channelData = m_bufferSource.getReadPointer(0);

		for (int i = 0; i < m_regions.size(); i++)
		{
			const float value = channelData[m_regions[i].m_sampleIndex];

			maxZeroCrossing = std::fmaxf(maxZeroCrossing, value);
		}

		m_maxZeroCrossingGainLabel.setText("Max. zero crossing: " + juce::String((float)juce::Decibels::gainToDecibels(maxZeroCrossing), 1), juce::dontSendNotification);

		updateValidZeroCrossingIdx();

		m_waveformDisplaySource.setRegions(m_regions);

		// Pass filtered buffer to waveform display when available
		m_waveformDisplaySource.setFilteredAudioBuffer(filteredBuffer, isFilterType);

		if (const size_t size = m_regions.size(); size > 1)
		{
			m_exportRegionLeftSlider.setRange(0.0, (double)(size - 1), 1.0);
			m_exportRegionRightSlider.setRange(0.0, (double)(size - 1), 1.0);
			m_exportRegionLeftSlider.setValue(0.0);
			m_exportRegionRightSlider.setValue((double)(size - 1));
		}
		else
		{
			m_exportRegionLeftSlider.setRange(0.0, 1.0, 1.0);
			m_exportRegionRightSlider.setRange(0.0, 1.0, 1.0);
			m_exportRegionLeftSlider.setValue(0.0);
			m_exportRegionRightSlider.setValue(0.0);
		}

		repaint();
	}

	//==========================================================================
	void sourceButtonClicked()
	{
		if (m_sourceType == SourceType::Source)
		{
			m_sourceButton.setButtonText("Output");
			m_sourceType = SourceType::Output;
		}
		else if (m_sourceType == SourceType::Output)
		{
			m_sourceButton.setButtonText("Source");
			m_sourceType = SourceType::Source;
		}
	}

	//==========================================================================
	int getMedian(std::vector<Region> regions)
	{
		if (regions.empty())
		{
			return 0;
		}

		std::vector<int> input;
		input.resize(regions.size());

		for (int i = 0; i < regions.size(); i++)
		{
			input[i] = regions[i].m_length;
		}

		return zazzDSP::Statistics::getMedian(input);
	}

	//==========================================================================
	juce::var serializeRegions()
	{
		auto regionsArray = std::make_unique<juce::DynamicObject>();
		juce::Array<juce::var> regionsVarArray;

		for (const auto& region : m_regions)
		{
			auto regionObject = std::make_unique<juce::DynamicObject>();
			regionObject->setProperty("sampleIndex", region.m_sampleIndex);
			regionObject->setProperty("isValid", region.m_isValid);
			regionObject->setProperty("length", region.m_length);
			regionObject->setProperty("difference", region.m_difference);

			regionsVarArray.add(juce::var(regionObject.release()));
		}

		return regionsVarArray;
	}

	//==========================================================================
	void deserializeRegions(const juce::var& regionsVar)
	{
		m_regions.clear();

		if (regionsVar.isArray())
		{
			const juce::Array<juce::var>* regionsArray = regionsVar.getArray();
			if (regionsArray != nullptr)
			{
				for (const auto& regionVar : *regionsArray)
				{
					if (regionVar.isObject())
					{
						auto* regionObj = regionVar.getDynamicObject();
						if (regionObj != nullptr)
						{
							Region region;
							region.m_sampleIndex = (int)regionObj->getProperty("sampleIndex");
							region.m_isValid = (bool)regionObj->getProperty("isValid");
							region.m_length = (int)regionObj->getProperty("length");
							region.m_difference = (float)regionObj->getProperty("difference");

							m_regions.push_back(region);
						}
					}
				}
			}
		}
	}

	//==========================================================================
	juce::var serializeRandomSelections()
	{
		juce::Array<juce::var> selectionsArray;

		for (const auto& selection : m_randomRegionSelections)
		{
			selectionsArray.add(selection);
		}

		return selectionsArray;
	}

	//==========================================================================
	void deserializeRandomSelections(const juce::var& selectionsVar)
	{
		m_randomRegionSelections.clear();

		if (selectionsVar.isArray())
		{
			const juce::Array<juce::var>* selectionsArray = selectionsVar.getArray();
			if (selectionsArray != nullptr)
			{
				for (const auto& selectionVar : *selectionsArray)
				{
					m_randomRegionSelections.push_back((int)selectionVar);
				}
			}
		}
	}

	//==========================================================================
	void clearRandomSelections()
	{
		m_randomRegionSelections.clear();
	}

	//==========================================================================
	void displayModeButtonClicked()
	{
		m_displayMode = (DisplayMode)((m_displayMode + 1) % 3);

		// Update spectrogram components with new display mode
		m_spectrogramDisplaySource.setDisplayMode((SpectrogramDisplayComponent::DisplayMode)m_displayMode);
		m_spectrogramDisplayOutput.setDisplayMode((SpectrogramDisplayComponent::DisplayMode)m_displayMode);

		switch (m_displayMode)
		{
			case DisplayMode::Waveform:
				m_displayModeButton.setButtonText("Waveform");
				m_waveformDisplaySource.setVisible(true);
				m_waveformDisplayOutput.setVisible(true);
				m_spectrogramDisplaySource.setVisible(false);
				m_spectrogramDisplayOutput.setVisible(false);
				break;

			case DisplayMode::Spectrogram:
				m_displayModeButton.setButtonText("Spectrogram");
				m_waveformDisplaySource.setVisible(false);
				m_waveformDisplayOutput.setVisible(false);
				m_spectrogramDisplaySource.setVisible(true);
				m_spectrogramDisplayOutput.setVisible(true);
				break;

			case DisplayMode::DominantFrequency:
				m_displayModeButton.setButtonText("Frequency");
				m_waveformDisplaySource.setVisible(false);
				m_waveformDisplayOutput.setVisible(false);
				m_spectrogramDisplaySource.setVisible(true);
				m_spectrogramDisplayOutput.setVisible(true);
				break;
		}
	}

	//==========================================================================
	// Buttons
	juce::TextButton m_openSourceButton;
	juce::TextButton m_detectRegionsButton;

	juce::TextButton m_generateButton;
	juce::TextButton m_saveButton;
	juce::TextButton m_exportRegionsButton;
	juce::TextButton m_saveProjectButton;
	juce::TextButton m_loadProjectButton;
	juce::TextButton m_newProjectButton;

	juce::TextButton m_playButton;
	juce::TextButton m_sourceButton;
	juce::TextButton m_displayModeButton;

	// Sliders
	juce::Slider m_thresholdSlider;
	juce::Slider m_minimumLengthSlider;
	juce::Slider m_SpectrumDifferenceSlider;
	juce::Slider m_fftPhaseThresholdSlider;
	juce::Slider m_zeroCrossingCountSlider;
	juce::Slider m_crossfadeLengthSlider;
	juce::Slider m_regionLenghtExportSlider;

	juce::Slider m_exportRegionLeftSlider;
	juce::Slider m_exportRegionRightSlider;
	juce::Slider m_exportMaxRegionOffsetSlider;
	juce::Slider m_exportRegionCountSlider;

	// Toggle button
	juce::ToggleButton m_spectrumMatchToggle;

	// Labels
	juce::Label m_sourceFileNameLabel;

	juce::Label m_detectedFrequencyLabel;
	juce::Label m_thresholdLabel;
	juce::Label m_maximumFrequencyLabel;
	juce::Label m_SpectrumDifferenceLabel;
	juce::Label m_fftPhaseThresholdLabel;
	juce::Label m_zeroCrossingCountLabel;
	juce::Label m_crossfadeLengthLabel;

	juce::Label m_regionLenghtMedianLabel;
	juce::Label m_regionLengthDiffLabel;
	juce::Label m_regionLenghtExportLabel;

	juce::Label m_regionsCountLabel;
	juce::Label m_validRegionsCountLabel;
	juce::Label m_maxZeroCrossingGainLabel;

	// Export
	juce::Label m_exportRegionLeftLabel;
	juce::Label m_exportRegionRightLabel;
	juce::Label m_exportMaxRegionOffsetLabel;
	juce::Label m_exportRegionCountLabel;
	juce::Label m_spectrumSourceLabel;

	zazzGUI::GroupLabel m_fileGroupLableComponent{ "File" };
	zazzGUI::GroupLabel m_sourceGroupLableComponent{ "Source" };
	zazzGUI::GroupLabel m_regionGroupLableComponent{ "Regions Detection" };
	zazzGUI::GroupLabel m_exportGroupLableComponent{ "Output" };
	zazzGUI::GroupLabel m_playbackGroupLableComponent{ "Playback" };
	zazzGUI::GroupLabel m_displayGroupLableComponent{ "Display" };

	// Combo boxes
	juce::ComboBox m_detectionTypeComboBox;
	juce::ComboBox m_generationTypeComboBox;
	juce::ComboBox m_spectrumSourceComboBox;

	//! Stores zero crossing points in source audio buffer
	std::vector<Region> m_regions{};

	//! Stores random region selections for reproducible generation
	std::vector<int> m_randomRegionSelections{};

	juce::AudioBuffer<float> m_bufferSource;
	juce::AudioBuffer<float> m_bufferOutput;

	WaveformDisplayComponent m_waveformDisplaySource;
	WaveformDisplayComponent m_waveformDisplayOutput;

	SpectrogramDisplayComponent m_spectrogramDisplaySource;
	SpectrogramDisplayComponent m_spectrogramDisplayOutput;

	// Spectrum matching
	std::unique_ptr<SpectrumMatchRegionProcessor> m_spectrumRegionProcessor;
	bool m_useSpectrumMatching = false;
	int m_selectedSpectrumRegionIndex = 0;


	float m_detectedFrequency = 0.0f;

	TransportState m_sourceState = TransportState::Stopped;
	SourceType m_sourceType = SourceType::Source;
	InterpolationType m_interpolationType = InterpolationType::Linear;
	bool m_showSpectrogram = false;
	DisplayMode m_displayMode = DisplayMode::Waveform;

	int m_regionLenghtMedian = 0;
	int m_playbackIndex = 0;
	int m_sampleRate = 48000;

	// From MainComponentBase
	juce::AudioFormatManager m_formatManager;
	juce::String m_fileName;						// TODO: Figure out how to pass it in callbacl
	juce::String m_sourceFilePath;					// Full path to loaded source file

	// File choosers
	std::unique_ptr<juce::FileChooser> m_wavOpenChooser;
	std::unique_ptr<juce::FileChooser> m_wavSaveChooser;
	std::unique_ptr<juce::FileChooser> m_projectSaveChooser;
	std::unique_ptr<juce::FileChooser> m_projectLoadChooser;
	std::unique_ptr<juce::FileChooser> m_regionsExportChooser;

	// Thread synchronization
	std::mutex m_bufferMutex;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};