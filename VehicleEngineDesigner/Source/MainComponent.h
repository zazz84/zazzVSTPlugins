#pragma once

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include <vector>
#include <algorithm>

#include "MainComponentBase.h"

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
class MainComponent  : public MainComponentBase
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    //void paint (juce::Graphics& g) override;
    void resized() override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override
	{
	}

	//==========================================================================
	void openSourceButtonClicked()
	{
		openFile(m_bufferSource, m_sampleRate, [this]()
		{
			// Draw waveform
			const float verticalZoom = 1.0f / m_bufferSource.getMagnitude(0, m_bufferSource.getNumSamples());
			m_waveformDisplaySource.setVerticalZoom(verticalZoom);
			m_waveformDisplaySource.setAudioBuffer(m_bufferSource);

			// Set filename label
			m_sourceFileNameLabel.setText(m_fileName, juce::dontSendNotification);
		});	
		
		/*m_chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", juce::File{}, "*.wav");
		auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

		m_chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
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
			});*/
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
		
		// Prepare out buffer
		const auto outputSize = validRegionsCount * exportRegionLength;
		const auto channels = m_bufferSource.getNumChannels();
		m_bufferOutput.setSize(channels, outputSize);

		for (int channel = 0; channel < channels; channel++)
		{
			auto* pBufferSource = m_bufferSource.getWritePointer(channel);
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
	}

	//==========================================================================
	void randomRegionsGenerate()
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

		// Temp audio buffer
		juce::AudioBuffer<float> tempBuffer;
		const int tempBufferLength = tempBufferRegionCount * exportRegionLength;
		const int channels = m_bufferSource.getNumChannels();
		tempBuffer.setSize(channels, tempBufferLength);
		
		for (int channel = 0; channel < channels; channel++)
		{
			auto* pBufferSource = m_bufferSource.getWritePointer(channel);
			auto* pTempBuffer = tempBuffer.getWritePointer(channel);
			int tempOutIndex = 0;

			for (int regionIdx = exportRegionLeftIdx; regionIdx < exportRegionRightIdx; regionIdx++)
			{
				if (m_regions[regionIdx].m_isValid == false)
				{
					continue;
				}

				const int segmentStartIndex = m_regions[regionIdx].m_sampleIndex;
				const int regionLenghtSource = m_regions[regionIdx].m_length;
				const float indexIncrement = (float)regionLenghtSource / (float)exportRegionLength;
				float sourceIndex = 0.0f;

				for (int i = 0; i < exportRegionLength; i++)
				{
					if (m_interpolationType == InterpolationType::Point)
					{
						pTempBuffer[tempOutIndex] = pBufferSource[segmentStartIndex + (int)sourceIndex];
					}
					else if (m_interpolationType == InterpolationType::Linear)
					{
						const int indexLeft = segmentStartIndex + (int)sourceIndex;
						const int indexRight = indexLeft < sourceSampleCount - 1 ? indexLeft + 1 : indexLeft;

						const float valueLeft = pBufferSource[indexLeft];
						const float valueRight = pBufferSource[indexRight];

						const float delta = sourceIndex - std::floor(sourceIndex);
						const float interpolated = valueLeft * (1.0f - delta) + valueRight * delta;

						pTempBuffer[tempOutIndex] = interpolated;

					}

					sourceIndex += indexIncrement;
					tempOutIndex++;
				}
			}
		}

		// Prepare out buffer
		m_bufferOutput.setSize(channels, exportRegionCount * exportRegionLength);
		m_bufferOutput.clear();

		// Writte to out buffer
		RandomNoRepeat randomNoRepeat(0, tempBufferRegionCount - 1, tempBufferRegionCount / 2);

		for (int outRegionIdx = 0; outRegionIdx < exportRegionCount; outRegionIdx++)
		{
			for (int channel = 0; channel < channels; channel++)
			{
				m_bufferOutput.copyFrom(channel, outRegionIdx * exportRegionLength, tempBuffer, channel, randomNoRepeat.get() * exportRegionLength, exportRegionLength);
			}
		}

		// Set output waveform
		std::vector<Region> regionsExport;
		
		regionsExport.resize(exportRegionCount);

		for (int i = 0; i < exportRegionCount; i++)
		{
			regionsExport[i].m_sampleIndex = i * exportRegionLength;
			regionsExport[i].m_isValid = true;
			regionsExport[i].m_length = exportRegionLength;

		}

		m_waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		m_waveformDisplayOutput.setRegions(regionsExport);
	}
	
	//==========================================================================
	void saveButtonClicked()
	{
		saveFile(m_bufferOutput, m_sampleRate);
		
		// Choose input file first (blocking for simplicity)
		/*chooser = std::make_unique<juce::FileChooser>("Save processed file as...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
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
							std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(stream.get(), m_sampleRate, m_bufferOutput.getNumChannels(), WRITTE_BIT_DEPTH, {}, 0));

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
			});*/
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
		zeroCrossing.set((float)m_detectedFrequencySlider.getValue(), 100, (float)juce::Decibels::decibelsToGain(m_thresholdSlider.getValue()), (float)m_maximumFrequencySlider.getValue());
		zeroCrossing.setType(m_detectionTypeComboBox.getSelectedId());

		std::vector<int> zeroCrossingIdxs;
		zeroCrossing.process(m_bufferSource, zeroCrossingIdxs);

		const size_t size = zeroCrossingIdxs.size();

		// Create regions
		m_regions.clear();
		
		if (size != 0)
		{
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
	
		m_regionsCountLabel.setText("Regions count: " + juce::String((float)size, 0), juce::dontSendNotification);

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
	juce::TextButton m_detectRegionsButton;
	
	juce::TextButton m_generateButton;
	juce::TextButton m_saveButton;

	juce::TextButton m_playButton;
	juce::TextButton m_sourceButton;

	// Sliders
	juce::Slider m_detectedFrequencySlider;
	juce::Slider m_thresholdSlider;
	juce::Slider m_maximumFrequencySlider;
	juce::Slider m_SpectrumDifferenceSlider;
	juce::Slider m_regionLenghtExportSlider;

	juce::Slider m_exportRegionLeftSlider;
	juce::Slider m_exportRegionRightSlider;
	juce::Slider m_exportMaxRegionOffsetSlider;
	juce::Slider m_exportRegionCountSlider;
	
	// Labels
	juce::Label m_sourceFileNameLabel;

	juce::Label m_detectedFrequencyLabel;
	juce::Label m_thresholdLabel;
	juce::Label m_maximumFrequencyLabel;
	juce::Label m_SpectrumDifferenceLabel;

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

	PluginNameComponent m_pluginNameComponent{ "zazz::VehicleEngineDesigner" };

	GroupLabelComponent m_sourceGroupLableComponent{ "Source" };
	GroupLabelComponent m_regionGroupLableComponent{ "Regions Detection" };
	GroupLabelComponent m_exportGroupLableComponent{ "Output" };
	GroupLabelComponent m_playbackGroupLableComponent{ "Playback" };

	// Combo boxes
	juce::ComboBox m_detectionTypeComboBox;
	juce::ComboBox m_generationTypeComboBox;

	//! Stores zero crossing points in source audio buffer
	std::vector<Region> m_regions{};
	
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