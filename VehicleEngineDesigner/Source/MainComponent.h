#pragma once

#include <JuceHeader.h>

#include <vector>
#include <algorithm>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
#include "../../../zazzVSTPlugins/Shared/GUI/WaveformDisplayComponent.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingRateOffline.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/ZeroCrossingOffline.h"

//==============================================================================
class BufferPositionableAudioSource : public juce::PositionableAudioSource
{
public:
	BufferPositionableAudioSource(const juce::AudioBuffer<float>& bufferToUse,
		bool shouldLoop = false)
		: buffer(bufferToUse), looping(shouldLoop)
	{
	}

	void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/) override
	{
		currentPosition = 0;
	}

	void releaseResources() override {}

	void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
	{
		auto totalSamples = buffer.getNumSamples();
		auto samplesNeeded = info.numSamples;
		auto samplesCopied = 0;

		while (samplesNeeded > 0)
		{
			if (currentPosition >= totalSamples)
			{
				if (looping)
					currentPosition = 0;
				else
				{
					info.buffer->clear(info.startSample + samplesCopied, samplesNeeded);
					return;
				}
			}

			auto samplesAvailable = totalSamples - currentPosition;
			auto toCopy = std::min(samplesAvailable, samplesNeeded);

			for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
			{
				if (ch < buffer.getNumChannels())
					info.buffer->copyFrom(ch, info.startSample + samplesCopied,
						buffer, ch, currentPosition, toCopy);
				else
					info.buffer->clear(ch, info.startSample + samplesCopied, toCopy);
			}

			currentPosition += toCopy;
			samplesCopied += toCopy;
			samplesNeeded -= toCopy;
		}
	}

	void setNextReadPosition(juce::int64 newPosition) override
	{
		currentPosition = (int)juce::jlimit((juce::int64)0,
			(juce::int64) buffer.getNumSamples(),
			newPosition);
	}

	juce::int64 getNextReadPosition() const override { return currentPosition; }

	juce::int64 getTotalLength() const override { return buffer.getNumSamples(); }

	bool isLooping() const override { return looping; }

	void setLooping(bool shouldLoop) { looping = shouldLoop; }

private:
	const juce::AudioBuffer<float>& buffer;
	int currentPosition{ 0 };
	bool looping{ false };
};

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
		if (source == &m_transportSource)
		{
			if (m_transportSource.isPlaying())
				changeState(Playing);
			else
				changeState(Stopped);
		}
	}

private:
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	void changeState(TransportState newState)
	{
		if (m_sourceState != newState)
		{
			m_sourceState = newState;

			switch (m_sourceState)
			{
			case Stopped:                           
				m_stopButton.setEnabled(false);
				m_playButton.setEnabled(true);
				m_transportSource.setPosition(0.0);
				break;

			case Starting:
				m_playButton.setEnabled(false);
				m_transportSource.start();
				break;

			case Playing:
				m_stopButton.setEnabled(true);
				break;

			case Stopping:
				m_transportSource.stop();
				break;
			}
		}
	}

	void openSourceButtonClicked()
	{
		chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", juce::File{}, "*.wav");
		auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

		chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
			{
				auto file = fc.getResult();

				if (file != juce::File{})                                               
				{
					auto* reader = formatManager.createReaderFor(file);                 

					if (reader != nullptr)
					{
						auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true); 
						m_transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
						m_playButton.setEnabled(true);                                                      
						m_sampleRate = (int)reader->sampleRate;
						readerSource.reset(newSource.release());                                         

						// Create audio buffer
						m_bufferSource.setSize((int)reader->numChannels, (int)reader->lengthInSamples);				
						reader->read(&m_bufferSource, 0, (int)reader->lengthInSamples, 0, true, true);
						m_bufferSourceMax = m_bufferSource.getMagnitude(0, m_bufferSource.getNumSamples());

						m_detectFrequencyButton.setEnabled(true);
					}

					waveformDisplaySource.loadFile(file);
				}
			});
	}

	void playSourceButtonClicked()
	{		
		changeState(Starting);
	}

	void stopSourceButtonClicked()
	{
		changeState(Stopping);
	}

	void generateButtonClicked()
	{
		generateOutput();
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
		ZeroCrossingRateOffline zeroCrossingRateOffline{};
		zeroCrossingRateOffline.init(m_sampleRate);
		zeroCrossingRateOffline.set(30, 80);

		m_detectedFrequency = 1.0f / zeroCrossingRateOffline.process(m_bufferSource);

		// Dislay detected frequency
		//m_detectedFrequencySlider.setText("Detected frequency: " + juce::String(m_detectedFrequency, 0) + " Hz", juce::dontSendNotification);
		m_detectedFrequencySlider.setValue(m_detectedFrequency);

		// Enable detect regions button
		m_detectRegionsButton.setEnabled(true);
	}

	void detectRegionsButtonClicked()
	{
		ZeroCrossingOffline zeroCrossing{};
		zeroCrossing.init(m_sampleRate);
		zeroCrossing.set(m_detectedFrequencySlider.getValue(), 100);

		zeroCrossing.process(m_bufferSource, m_regions);
		
		m_regionsCountLabel.setText("Regions count: " + juce::String((float)m_regions.size(), 0), juce::dontSendNotification);

		// Get median
		std::vector<int> diff{};
		diff.resize(m_regions.size() - 1);

		for (int i = 0; i < m_regions.size() - 1; i++)
		{
			diff[i] = m_regions[i + 1] - m_regions[i];
		}

		const int median = getMedian(diff);

		m_regionLenghtSlider.setValue((float)median);

		repaint();
	}

	//==========================================================================
	void generateOutput()
	{
		// Resample
		const int regionsCount = m_validRegionsIdx.size();
		const int regionLenght = (int)m_regionLenghtSlider.getValue();
		const int regionLenghtExport = (int)m_regionLenghtExportSlider.getValue();
		const int regionOffset = (int)m_regionOffsetLenghtSlider.getValue();
		//const int regionLenght = 1000;

		// Prepare out buffer
		m_bufferOutput.setSize(m_bufferSource.getNumChannels(), regionsCount * regionLenghtExport);
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

		const int regionMedian = (int)m_regionLenghtSlider.getValue();
		const int regionOffset = (int)m_regionOffsetLenghtSlider.getValue();
		const int leftThreshold = regionMedian - regionOffset;
		const int rightThreshold = regionMedian + regionOffset;

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

		g.setColour(juce::Colours::red);

		const int samples = m_bufferSource.getNumSamples();
		const int width = getWidth();
		const float factor = (float)width / (float)samples;

		const int top = 350;																	  
		const int bottom = 450;

		// Draw all regions
		g.setColour(juce::Colours::red);

		for (int region = 0; region < m_regions.size() - 1; region++)
		{
			const int x = factor * m_regions[region];                    
														      
			g.drawLine((float)x, (float)top, (float)x, (float)bottom, 1.0f);
		}
		
		// Draw valid regions
		g.setColour(juce::Colours::green);

		for (int id = 0; id < m_validRegionsIdx.size() - 1; id++)
		{
			const int x = factor * m_regions[m_validRegionsIdx[id]];											

			g.drawLine((float)x, (float)top, (float)x, (float)bottom, 3.0f);
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
	juce::TextButton m_stopButton;

	juce::Slider m_detectedFrequencySlider;
	juce::Slider m_regionLenghtSlider;
	juce::Slider m_regionOffsetLenghtSlider;
	juce::Slider m_regionLenghtExportSlider;
	
	juce::Label m_detectedFrequencyLabel;
	juce::Label m_regionLenghtLabel;
	juce::Label m_regionOffsetLenghtLabel;
	juce::Label m_regionLenghtExportLabel;
	
	juce::Label m_regionsCountLabel;
	juce::Label m_validRegionsCountLabel;

	//! Stores zero crossing points in source audio buffer
	std::vector<int> m_regions{};

	//! Stores valid indexes to zero crossing points
	std::vector<int> m_validRegionsIdx{};

	std::unique_ptr<juce::FileChooser> chooser;

	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource m_transportSource;
	
	juce::AudioBuffer<float> m_bufferSource;

	juce::AudioBuffer<float> m_bufferOutput;
	
	TransportState m_sourceState = TransportState::Stopped;
	int m_playbackIndex = 0;
	
	int m_sampleRate = 48000;

	float m_bufferSourceMax = 0.0f;
	float m_detectedFrequency = 0.0f;
	int m_greenRegionsCount = 0;
	
	WaveformDisplayComponent waveformDisplaySource;
	WaveformDisplayComponent waveformDisplayOutput;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};