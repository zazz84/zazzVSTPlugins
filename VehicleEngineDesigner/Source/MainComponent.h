#pragma once

#include <JuceHeader.h>

#include <vector>
#include <algorithm>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/GUI/WaveformDisplayComponent.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingRateOffline.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingOffline.h"


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
							waveformDisplaySource.setVerticalZoom(verticalZoom);
							waveformDisplaySource.setAudioBuffer(m_bufferSource);
						}

						m_sourceFileNameLabel.setText(file.getFileName(), juce::dontSendNotification);
					}
				}
			});
	}

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

	void generateButtonClicked()
	{
		generateOutput();

		if (m_bufferOutput.getNumSamples() != 0)
		{
			const float verticalZoom = 1.0f / m_bufferOutput.getMagnitude(0, m_bufferOutput.getNumSamples());
			waveformDisplayOutput.setVerticalZoom(verticalZoom);
			waveformDisplayOutput.setAudioBuffer(m_bufferOutput);
		}
	}

	void saveButtonClicked()
	{
		// Choose input file first (blocking for simplicity)
		chooser = std::make_unique<juce::FileChooser>("Save processed file as...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
		int flags = juce::FileBrowserComponent::saveMode;

		chooser->launchAsync(flags, [this](const juce::FileChooser& fc)
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

	void detectRegionsButtonClicked()
	{		
		ZeroCrossingOffline zeroCrossing{};
		zeroCrossing.init(m_sampleRate);
		zeroCrossing.set(m_detectedFrequencySlider.getValue(), 100);
		zeroCrossing.setType(m_detectionTypeComboBox.getSelectedId());

		zeroCrossing.process(m_bufferSource, m_regions);
		
		m_regionsCountLabel.setText("Regions count: " + juce::String((float)m_regions.size(), 0), juce::dontSendNotification);

		// Get median
		std::vector<int> diff{};
		diff.resize(m_regions.size() - 1);

		for (int i = 0; i < m_regions.size() - 1; i++)
		{
			diff[i] = m_regions[i + 1] - m_regions[i];
		}

		m_regionLenghtMedian = getMedian(diff);

		m_regionLenghtMedianLabel.setText("Region Lenght Median: " + juce::String((float)m_regionLenghtMedian, 0), juce::dontSendNotification);

		repaint();
	}

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
	void generateOutput()
	{
		// Resample
		const int regionsCount = m_validRegionsIdx.size();
		const int regionLenghtExport = (int)m_regionLenghtExportSlider.getValue();
		const int regionOffset = (int)m_regionOffsetLenghtSlider.getValue();
		const int size = regionsCount * regionLenghtExport;

		// Prepare out buffer
		m_bufferOutput.setSize(m_bufferSource.getNumChannels(), size);
		m_bufferOutput.clear();

		// TODO: Handling for stereo
		auto* pBufferSource = m_bufferSource.getWritePointer(0);
		auto* pBufferOut = m_bufferOutput.getWritePointer(0);

		int outIndex = 0;

		for (int region = 0; region < regionsCount; region++)
		{
			const int regionIdx = m_validRegionsIdx[region];
			const int segmentStartIndex = m_regions[regionIdx];
			const int regionLenghtSource = m_regions[regionIdx + 1] - segmentStartIndex;
			const float indexIncrement = (float)regionLenghtSource / (float)regionLenghtExport;

			float sourceIndex = 0.0f;

			for (int i = 0; i < regionLenghtExport; i++)
			{							
				// TODO: Add linear interpolation
				/*const int l = segmentStartIndex + (int)sourceIndex;
				const int r = std::min(l + 1, size - 1);
				const float d = sourceIndex - (int)sourceIndex;

				const float out = pBufferSource[l] * (1.0f - d) + pBufferSource[r] * d;
				pBufferOut[outIndex] = out;*/

				pBufferOut[outIndex] = pBufferSource[segmentStartIndex + (int)sourceIndex];

				sourceIndex += indexIncrement;
				outIndex++;
			}
		}
	}

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
	}

	void drawRegions(juce::Graphics& g)
	{
		if (m_regions.size() == 0)
		{
			return;
		}

		const int samples = m_bufferSource.getNumSamples();
		const int width = getWidth() - 20;
		const float factor = (float)width / (float)samples;

		const int top = 540;																	  
		const int bottom = top + 160;

		// Draw all regions
		g.setColour(juce::Colours::red);

		for (int region = 0; region < m_regions.size() - 1; region++)
		{
			const int x = factor * m_regions[region];                    
														      
			g.drawLine((float)x + 10.0f, (float)top, (float)x + 10.0f, (float)bottom, 1.0f);
		}
		
		// Draw valid regions
		g.setColour(juce::Colours::whitesmoke);

		for (int id = 0; id < m_validRegionsIdx.size() - 1; id++)
		{
			const int x = factor * m_regions[m_validRegionsIdx[id]];											

			g.drawLine((float)x + 10.0f, (float)top, (float)x + 10.0f, (float)bottom, 3.0f);
		}
	}

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
	juce::Slider m_regionOffsetLenghtSlider;
	juce::Slider m_regionLenghtExportSlider;
	
	// Labels
	juce::Label m_sourceFileNameLabel;
	juce::Label m_detectedFrequencyLabel;
	juce::Label m_regionLenghtMedianLabel;
	juce::Label m_regionOffsetLenghtLabel;
	juce::Label m_regionLenghtExportLabel;
	
	juce::Label m_regionsCountLabel;
	juce::Label m_validRegionsCountLabel;

	// Combo boxes
	juce::ComboBox m_detectionTypeComboBox;

	//! Stores zero crossing points in source audio buffer
	std::vector<int> m_regions{};

	//! Stores valid indexes to zero crossing points
	std::vector<int> m_validRegionsIdx{};

	std::unique_ptr<juce::FileChooser> chooser;

	juce::AudioFormatManager m_formatManager;
	
	juce::AudioBuffer<float> m_bufferSource;
	juce::AudioBuffer<float> m_bufferOutput;

	WaveformComponent waveformDisplaySource;
	WaveformComponent waveformDisplayOutput;

	float m_detectedFrequency = 0.0f;

	TransportState m_sourceState = TransportState::Stopped;
	SourceType m_sourceType = SourceType::Source;

	int m_regionLenghtMedian = 0;
	int m_playbackIndex = 0;
	int m_sampleRate = 48000;
	

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};