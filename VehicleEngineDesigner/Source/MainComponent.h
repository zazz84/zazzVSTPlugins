#pragma once

#include <JuceHeader.h>

#include <vector>
#include <algorithm>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/GUI/WaveformDisplayComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/PluginNameComponent.h"
#include "../../../zazzVSTPlugins/Shared/GUI/GroupLabelComponent.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingRateOffline.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingOffline.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/RandomNoRepeat.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

	static const int CANVAS_WIDTH = 1 + 15 + 1 + 15 + 1;
	static const int CANVAS_HEIGHT = 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 11 + 11 + 1;

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

private:
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

	//==========================================================================
	void openSourceButtonClicked()
	{
		chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", juce::File{}, "*.wav");
		auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

		chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
			{
				auto file = fc.getResult();

				if (file != juce::File{})                                               
				{
					auto* reader = m_formatManager.createReaderFor(file);                 

					if (reader != nullptr)
					{                                                  
						m_sampleRate = (int)reader->sampleRate;                                        

						// Create audio buffer
						m_bufferSource.setSize((int)reader->numChannels, (int)reader->lengthInSamples);				
						reader->read(&m_bufferSource, 0, (int)reader->lengthInSamples, 0, true, true);

						// Draw waveform
						if (m_bufferSource.getNumSamples() != 0)
						{
							const float verticalZoom = 1.0f / m_bufferSource.getMagnitude(0, m_bufferSource.getNumSamples());
							m_waveformDisplaySource.setVerticalZoom(verticalZoom);
							m_waveformDisplaySource.setAudioBuffer(m_bufferSource);

							//m_regionsComponent.setHorizontalZoom(m_zoomRegionLeftLabel.getText().getIntValue(), m_zoomRegionRightLabel.getText().getIntValue());
						}

						m_sourceFileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
					}
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
		if (static_cast<GenerationType>(m_generationTypeComboBox.getSelectedId()) == GenerationType::RandomRegion)
		{
			randomRegionsGenerate();
		}
		else
		{
			flatRefionsGenerate();
		}
	}

	//==========================================================================
	void flatRefionsGenerate()
	{
		const auto validRegionsCount = (int)m_validRegionsIdx.size();
		if (validRegionsCount == 0)
		{
			return;
		}

		const auto exportRegionLength = (int)m_regionLenghtExportSlider.getValue();
		if (exportRegionLength == 0)
		{
			return;
		}

		const auto sourceSize = m_bufferSource.getNumSamples();
		if (sourceSize == 0)
		{
			return;
		}
		
		// Prepare out buffer
		const auto outputSize = validRegionsCount * exportRegionLength;
		m_bufferOutput.setSize(m_bufferSource.getNumChannels(), outputSize);

		// TODO: Handling for stereo
		auto* pBufferSource = m_bufferSource.getWritePointer(0);
		auto* pBufferOut = m_bufferOutput.getWritePointer(0);

		int outIndex = 0;

		for (int region = 0; region < validRegionsCount; region++)
		{
			const int regionIdx = m_validRegionsIdx[region];
			const int segmentStartIndex = m_regions[regionIdx];
			const int sourceRegionLength = m_regions[regionIdx + 1] - segmentStartIndex;
			const float indexIncrement = (float)sourceRegionLength / (float)exportRegionLength;

			float sourceIndex = 0.0f;

			for (int i = 0; i < exportRegionLength; i++)
			{
				if (m_interpolationType == InterpolationType::Point)
				{
					pBufferOut[outIndex] = pBufferSource[segmentStartIndex + (int)sourceIndex];
				}
				else if (m_interpolationType == InterpolationType::Linear)
				{
					const int indexLeft = segmentStartIndex + (int)sourceIndex;
					const int indexRight = indexLeft < sourceSize - 1 ? indexLeft + 1 : indexLeft;

					const float valueLeft = pBufferSource[indexLeft];
					const float valueRight = pBufferSource[indexRight];

					const float delta = sourceIndex - std::floor(sourceIndex);
					const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

					pBufferOut[outIndex] = interpolated;
				}

				sourceIndex += indexIncrement;
				outIndex++;
			}
		}

		// Set output waveform
		std::vector<int> regionsExport;
		std::vector<int> validRegionExportIdx;

		regionsExport.resize(validRegionsCount + 1);
		validRegionExportIdx.resize(validRegionsCount);

		int value = 0;
		for (int i = 0; i < validRegionsCount; i++)
		{
			regionsExport[i] = value;
			validRegionExportIdx[i] = i;
			value += exportRegionLength;
		}

		regionsExport[validRegionsCount] = value;

		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		m_waveformDisplayOutput.setRegions(regionsExport, validRegionExportIdx);
	}

	//==========================================================================
	void randomRegionsGenerate()
	{
		const auto exportRegionLeftIdx = (int)m_exportRegionLeftSlider.getValue();
		const auto exportRegionRightIdx = (int)m_exportRegionRightSlider.getValue();
		const auto exportRegionCout = (int)m_exportRegionCountSlider.getValue();

		const auto sourceRegionCount = (int)m_regions.size() - 1;
		const auto sourceSampleCount = m_bufferSource.getNumSamples();
		const auto exportRegionLength = (int)m_regionLenghtExportSlider.getValue();

		// Check if setup is valid
		if (exportRegionLeftIdx >= sourceRegionCount &&
			exportRegionLeftIdx < 0 &&
			exportRegionRightIdx > sourceRegionCount &&
			exportRegionRightIdx < 1 &&
			exportRegionCout <= 0)
		{
			return;
		}

		const GenerationType generationType = static_cast<GenerationType>(m_generationTypeComboBox.getSelectedId());

		auto* pBufferSource = m_bufferSource.getWritePointer(0);

		// Temp audio buffer
		juce::AudioBuffer<float> tempBuffer;
		const int tempBufferRegionCount = exportRegionRightIdx - exportRegionLeftIdx;
		const int tempBufferLength = tempBufferRegionCount * exportRegionLength;
		tempBuffer.setSize(m_bufferSource.getNumChannels(), tempBufferLength);
		auto* pBufferTemp = tempBuffer.getWritePointer(0);

		int tempOutIndex = 0;

		for (int regionIdx = exportRegionLeftIdx; regionIdx < exportRegionRightIdx; regionIdx++)
		{
			const int segmentStartIndex = m_regions[regionIdx];
			const int regionLenghtSource = m_regions[regionIdx + 1] - segmentStartIndex;
			const float indexIncrement = (float)regionLenghtSource / (float)exportRegionLength;
			float sourceIndex = 0.0f;

			for (int i = 0; i < exportRegionLength; i++)
			{
				if (m_interpolationType == InterpolationType::Point)
				{
					pBufferTemp[tempOutIndex] = pBufferSource[segmentStartIndex + (int)sourceIndex];
				}
				else if (m_interpolationType == InterpolationType::Linear)
				{
					const int indexLeft = segmentStartIndex + (int)sourceIndex;
					const int indexRight = indexLeft < sourceSampleCount - 1 ? indexLeft + 1 : indexLeft;

					const float valueLeft = pBufferSource[indexLeft];
					const float valueRight = pBufferSource[indexRight];

					const float delta = sourceIndex - std::floor(sourceIndex);
					const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

					pBufferTemp[tempOutIndex] = interpolated;

				}

				sourceIndex += indexIncrement;
				tempOutIndex++;
			}
		}

		// Prepare out buffer
		m_bufferOutput.setSize(m_bufferSource.getNumChannels(), exportRegionCout * exportRegionLength);
		m_bufferOutput.clear();

		// TODO: Handling for stereo		
		auto* pBufferOut = m_bufferOutput.getWritePointer(0);

		RandomNoRepeat randomNoRepeat(0, tempBufferRegionCount - 1, tempBufferRegionCount / 2);

		for (int outRegionIdx = 0; outRegionIdx < exportRegionCout; outRegionIdx++)
		{
			m_bufferOutput.copyFrom(0, outRegionIdx * exportRegionLength, tempBuffer, 0, randomNoRepeat.get() * exportRegionLength, exportRegionLength);
		}

		// Set output waveform
		std::vector<int> regionsExport;
		std::vector<int> validRegionsExportIdx;
		
		regionsExport.resize(exportRegionCout + 1);
		validRegionsExportIdx.resize(exportRegionCout);

		int value = 0;
		for (int i = 0; i < exportRegionCout; i++)
		{
			regionsExport[i] = value;
			validRegionsExportIdx[i] = i;
			value += exportRegionLength;
		}

		regionsExport[exportRegionCout] = value;

		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		m_waveformDisplayOutput.setRegions(regionsExport, validRegionsExportIdx);
	}
	
	//==========================================================================
	void saveButtonClicked()
	{
		// Choose input file first (blocking for simplicity)
		chooser = std::make_unique<juce::FileChooser>("Save processed file as...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
		int flag = juce::FileBrowserComponent::saveMode;

		chooser->launchAsync(flag, [this](const juce::FileChooser& fc)
			{
				juce::File outFile = fc.getResult();
				if (outFile == juce::File()) // user cancelled
					return;

				if (outFile.getFileExtension().isEmpty())
					outFile = outFile.withFileExtension(".wav");

			
				// Save on background thread
				std::thread([this, outFile]()
					{
						juce::WavAudioFormat wavFormat;
						std::unique_ptr<juce::FileOutputStream> stream(outFile.createOutputStream());

						bool ok = false;
						if (stream != nullptr)
						{
							std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(stream.get(), m_sampleRate, m_bufferOutput.getNumChannels(), 16, {}, 0));

							if (writer != nullptr)
							{
								stream.release(); // writer owns stream
								ok = writer->writeFromAudioSampleBuffer(m_bufferOutput, 0, m_bufferOutput.getNumSamples());
							}
						}

						// Notify on UI thread
						juce::MessageManager::callAsync([outFile, ok]()
							{
								if (ok)
									juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
										"Saved",
										"Processed file saved to:\n" + outFile.getFullPathName());
								else
									juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
										"Error",
										"Failed to save file!");
							});
					}).detach();
			});
	}

	//==========================================================================
	void detectFrequencyButtonClicked()
	{
		if (m_bufferSource.getNumSamples() == 0)
		{
			return;
		}
		
		ZeroCrossingRateOffline zeroCrossingRateOffline{};
		zeroCrossingRateOffline.init(m_sampleRate);
		zeroCrossingRateOffline.set(2, 200);

		m_detectedFrequency = 1.0f / zeroCrossingRateOffline.process(m_bufferSource);

		m_detectedFrequencySlider.setValue(m_detectedFrequency);
	}

	//==========================================================================
	void updateValidZeroCrossingIdx()
	{
		if (m_regions.empty())
		{
			return;
		}

		m_validRegionsIdx.clear();

		const int regionOffset = (int)m_regionOffsetLenghtSlider.getValue();
		const int leftThreshold = m_regionLenghtMedian - regionOffset;
		const int rightThreshold = m_regionLenghtMedian + regionOffset;

		m_validRegionsIdx.push_back(0);

		for (int region = 1; region < m_regions.size() - 1; region++)
		{
			const int regionLenght = m_regions[region + 1] - m_regions[region];

			if (regionLenght < leftThreshold || leftThreshold > rightThreshold)
			{
				continue;
			}

			m_validRegionsIdx.push_back(region);
		}

		m_validRegionsCountLabel.setText("Valid regions count: " + juce::String((float)m_validRegionsIdx.size(), 0), juce::dontSendNotification);

		m_waveformDisplaySource.setRegions(m_regions, m_validRegionsIdx);
	}

	//==========================================================================
	void detectRegionsButtonClicked()
	{		
		ZeroCrossingOffline zeroCrossing{};
		zeroCrossing.init(m_sampleRate);
		zeroCrossing.set((float)m_detectedFrequencySlider.getValue(), 100, (float)juce::Decibels::decibelsToGain(m_thresholdSlider.getValue()), (float)m_maximumFrequencySlider.getValue());
		zeroCrossing.setType(m_detectionTypeComboBox.getSelectedId());

		zeroCrossing.process(m_bufferSource, m_regions);
		
		m_regionsCountLabel.setText("Count: " + juce::String((float)m_regions.size() - 1, 0), juce::dontSendNotification);

		// Get median
		std::vector<int> diff{};
		diff.resize(m_regions.size() - 1);

		for (int i = 0; i < m_regions.size() - 1; i++)
		{
			diff[i] = m_regions[i + 1] - m_regions[i];
		}

		m_regionLenghtMedian = getMedian(diff);

		m_regionLenghtMedianLabel.setText("Lenght median: " + juce::String((float)m_regionLenghtMedian, 0), juce::dontSendNotification);

		int maxDiff = 0;

		for (int i = 0; i < diff.size() - 1; i++)
		{
			const int medianDiff = std::abs(m_regionLenghtMedian - diff[i]);

			if (medianDiff > maxDiff)
			{
				maxDiff = medianDiff;
			}
		}

		m_regionLengthDiffLabel.setText("Max median diff: " + juce::String((float)maxDiff, 0), juce::dontSendNotification);

		m_regionLenghtExportSlider.setValue(m_regionLenghtMedian);

		// Handle max zero crossing
		float maxZeroCrossing = 0.0f;
		auto* channelData = m_bufferSource.getReadPointer(0);

		for (int i = 0; i < m_regions.size(); i++)
		{
			const float value = channelData[m_regions[i]];
			
			maxZeroCrossing = std::fmaxf(maxZeroCrossing, value);
		}

		m_maxZeroCrossingGainLabel.setText("Max. zero crossing: " + juce::String((float)juce::Decibels::gainToDecibels(maxZeroCrossing), 1), juce::dontSendNotification);

		updateValidZeroCrossingIdx();

		m_waveformDisplaySource.setRegions(m_regions, m_validRegionsIdx);

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
	int getMedian(std::vector<int> input)
	{
		if (input.empty())
		{
			return 0;
		}

		std::sort(input.begin(), input.end());
		size_t n = input.size();

		if (n % 2 == 1) // odd number of elements
			return input[n / 2];
		else // even number of elements
			return (input[n / 2 - 1] + input[n / 2]) / 2;
	}
	
	//==========================================================================
	// Buttons
	juce::TextButton m_openSourceButton;
	juce::TextButton m_detectFrequencyButton;
	juce::TextButton m_detectRegionsButton;
	
	juce::TextButton m_generateButton;
	juce::TextButton m_saveButton;

	juce::TextButton m_playButton;
	juce::TextButton m_sourceButton;

	// Sliders
	juce::Slider m_detectedFrequencySlider;
	juce::Slider m_thresholdSlider;
	juce::Slider m_maximumFrequencySlider;
	juce::Slider m_regionOffsetLenghtSlider;
	juce::Slider m_regionLenghtExportSlider;

	juce::Slider m_exportRegionLeftSlider;
	juce::Slider m_exportRegionRightSlider;
	juce::Slider m_exportRegionCountSlider;
	
	// Labels
	juce::Label m_sourceFileNameLabel;

	juce::Label m_detectedFrequencyLabel;
	juce::Label m_thresholdLabel;
	juce::Label m_maximumFrequencyLabel;

	juce::Label m_regionLenghtMedianLabel;
	juce::Label m_regionLengthDiffLabel;
	juce::Label m_regionOffsetLenghtLabel;
	juce::Label m_regionLenghtExportLabel;
	
	juce::Label m_regionsCountLabel;
	juce::Label m_validRegionsCountLabel;
	juce::Label m_maxZeroCrossingGainLabel;

	// Export
	juce::Label m_exportRegionLeftLabel;
	juce::Label m_exportRegionRightLabel;
	juce::Label m_exportRegionCountLabel;

	PluginNameComponent m_pluginNameComponent{ "zazz::VehicleEngineDesigner" };

	GroupLabelComponent m_sourceGroupLableComponent{ "Source" };
	GroupLabelComponent m_regionGroupLableComponent{ "Regions Detection" };
	GroupLabelComponent m_exportGroupLableComponent{ "Output" };
	GroupLabelComponent m_playbackGroupLableComponent{ "Playback" };

	// Combo boxes
	juce::ComboBox m_detectionTypeComboBox;
	juce::ComboBox m_generationTypeComboBox;

	//! Stores zero crossing points in source audio buffer
	std::vector<int> m_regions{};

	//! Stores valid indexes to zero crossing points
	std::vector<int> m_validRegionsIdx{};

	std::unique_ptr<juce::FileChooser> chooser;

	juce::AudioFormatManager m_formatManager;
	
	juce::AudioBuffer<float> m_bufferSource;
	juce::AudioBuffer<float> m_bufferOutput;

	WaveformDisplayComponent m_waveformDisplaySource;
	WaveformDisplayComponent m_waveformDisplayOutput;

	float m_detectedFrequency = 0.0f;

	TransportState m_sourceState = TransportState::Stopped;
	SourceType m_sourceType = SourceType::Source;
	InterpolationType m_interpolationType = InterpolationType::Linear;

	int m_regionLenghtMedian = 0;
	int m_playbackIndex = 0;
	int m_sampleRate = 48000;

	// Colors
	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};