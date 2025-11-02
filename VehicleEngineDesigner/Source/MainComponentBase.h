#pragma once

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include <vector>
#include <algorithm>

#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"
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
class MainComponentBase : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
	//==============================================================================
	MainComponentBase();
	~MainComponentBase() override;

	static const int CANVAS_WIDTH = 1 + 15 + 1 + 15;
	static const int CANVAS_HEIGHT = 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 11 + 11 + 1 + 1;
	static const int PIXEL_SIZE = 27;
	static const int WRITTE_BIT_DEPTH = 32;		// 32-bit float

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(juce::Graphics& g) override;
	void resized() override;

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

	juce::AudioFormatManager m_formatManager;
	std::unique_ptr<juce::FileChooser> m_chooser;
	juce::String m_fileName;						// TODO: Figure out how to pass it in callbacl

	// Colors
	juce::Colour darkColor = juce::Colour::fromRGB(40, 42, 46);
	juce::Colour lightColor = juce::Colour::fromRGB(68, 68, 68);
	juce::Colour highlightColor = juce::Colour::fromRGB(255, 255, 190);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponentBase)
};