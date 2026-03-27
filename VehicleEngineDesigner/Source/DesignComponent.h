#pragma once

#include <JuceHeader.h>

#include <vector>

#include "../../../zazzVSTPlugins/Shared/GUI/WaveformRuntimeComponent.h"
#include "../../../zazzVSTPlugins/Shared/Filters/SpectrumMatch.h"

//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
class DesignComponent : public juce::AudioAppComponent, public juce::Timer, public juce::ChangeListener
{
public:
	//==============================================================================
	DesignComponent();
	~DesignComponent() override;

	static const int SOURCE_COUNT = 6;
	static const int CANVAS_WIDTH = 1 + 15 + 1 + 15;
	static const int CANVAS_HEIGHT = 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 11 + 11 + 1 + 1;
	static const int PIXEL_SIZE = 27;
	static const int WRITTE_BIT_DEPTH = 32;		// 32-bit float

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	void timerCallback() override;

	//==============================================================================
	void resized() override;
	void paint(juce::Graphics& g) override;

	void changeListenerCallback(juce::ChangeBroadcaster* source) override
	{
	}

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

	//==========================================================================
	void openFile(juce::AudioBuffer<float>& buffer, int& sampleRate, std::function<void()> onLoaded = nullptr)
	{
		m_chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...", juce::File{}, "*.wav");
		auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

		m_chooser->launchAsync(chooserFlags, [this, &buffer, &sampleRate, onLoaded](const juce::FileChooser& fc)
		{
			auto file = fc.getResult();

			if (file != juce::File{})
			{
				// Set file name
				m_fileName = file.getFileName();

				auto* reader = m_formatManager.createReaderFor(file);

				if (reader != nullptr)
				{
					const int samples = (int)reader->lengthInSamples;

					if (samples != 0)
					{
						// Set sample rate
						sampleRate = static_cast<int>(reader->sampleRate);

						// Create audio buffer
						buffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
						reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);

						// Callback when file is loaded
						if (onLoaded)
						{
							juce::MessageManager::callAsync(onLoaded);
						}
					}
				}
			}
		});
	}

	//==========================================================================
	void saveFile(juce::AudioBuffer<float>& buffer, int sampleRate)
	{
		// Choose input file first (blocking for simplicity)
		m_chooser = std::make_unique<juce::FileChooser>("Save processed file as...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
		int flag = juce::FileBrowserComponent::saveMode;

		m_chooser->launchAsync(flag, [this, &buffer, sampleRate](const juce::FileChooser& fc)
		{
			juce::File outFile = fc.getResult();
			if (outFile == juce::File()) // user cancelled
				return;

			if (outFile.getFileExtension().isEmpty())
				outFile = outFile.withFileExtension(".wav");


			// Save on background thread
			std::thread([this, outFile, &buffer, sampleRate]()
			{
				juce::WavAudioFormat wavFormat;
				std::unique_ptr<juce::FileOutputStream> stream(outFile.createOutputStream());

				bool ok = false;
				if (stream != nullptr)
				{
					std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(stream.get(), sampleRate, buffer.getNumChannels(), WRITTE_BIT_DEPTH, {}, 0));

					if (writer != nullptr)
					{
						stream.release(); // writer owns stream
						ok = writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
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

private:
	int getUsedSourcesCount()
	{
		// Get number of defined sources
		int usedSources = SOURCE_COUNT;
		for (int i = SOURCE_COUNT - 1; i >= 0; i--)
		{
			if (m_bufferSource[i].getNumSamples() == 0)
			{
				usedSources = i;
			}
		}

		return usedSources;
	}
	void resetPlaybackIndex()
	{
		for (size_t i = 0; i < SOURCE_COUNT; i++)
		{
			for (size_t j = 0; j < 2; j++)
			{
				m_playbackIndex[i][j] = 0.0f;
			}
		}
	}

	//==============================================================================
	WaveformRuntimeComponent m_waveformDisplay;
	
	// Buttons
	juce::TextButton m_openSourceButton[SOURCE_COUNT];
	juce::TextButton m_playSourceButton;
	juce::TextButton m_playProcessedButton;
	juce::TextButton m_applySpectrumMatchButton;

	// Labels
	juce::Label m_sourceFileNameLabel[SOURCE_COUNT];
	juce::Label m_regionLength[SOURCE_COUNT];
	juce::Label m_regionCrossfade[SOURCE_COUNT];
	juce::Label m_gainLabel[SOURCE_COUNT];
	juce::Label m_regionLengthPlaybackLabel;

	// Sliders
	juce::Slider m_regionLengthPlaybackSlider;
	juce::Slider m_spectrumMatchSlider[SOURCE_COUNT];

	// Audio buffers
	juce::AudioBuffer<float> m_bufferSource[SOURCE_COUNT];
	juce::AudioBuffer<float> m_bufferProcessed[SOURCE_COUNT];
	int m_sampleRate[SOURCE_COUNT];
	float m_playbackIndex[SOURCE_COUNT][2] = { 0.0f };
	float m_gain[SOURCE_COUNT] = { 0.0f };

	// Misc
	TransportState m_sourceState = TransportState::Stopped;
	int m_usedSources;
	bool m_playSource = true;

	// From MainComponentBase
	juce::AudioFormatManager m_formatManager;
	std::unique_ptr<juce::FileChooser> m_chooser;
	juce::String m_fileName;						// TODO: Figure out how to pass it in callbacl

	// Colors
	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DesignComponent)
};