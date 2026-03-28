#pragma once

#include <JuceHeader.h>
#include <memory>
#include <functional>

namespace zazzDSP
{
    class FileIO
    {
    public:
        static void openWavFile(
            juce::AudioBuffer<float>& buffer,
            int& sampleRate,
            juce::AudioFormatManager& formatManager,
            juce::String& fileFullPathName,
            std::unique_ptr<juce::FileChooser>& chooser,
            std::function<void()> onLoaded = nullptr)
        {
            chooser = std::make_unique<juce::FileChooser>(
                "Select a Wave file ...",
                juce::File{},
                "*.wav");

            auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

            chooser->launchAsync(chooserFlags, [&buffer, &sampleRate, &formatManager, &fileFullPathName, onLoaded](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();

                if (file != juce::File{})
                {
                    // Set file name
                    fileFullPathName = file.getFullPathName();

                    auto* reader = formatManager.createReaderFor(file);

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
        static void saveWavFile(
            juce::AudioBuffer<float>& buffer,
            int sampleRate,
            std::unique_ptr<juce::FileChooser>& chooser,
            int bitDepth = 32)
        {
            chooser = std::make_unique<juce::FileChooser>(
                "Save processed Wav file as...",
                juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                "*.wav");

            int flag = juce::FileBrowserComponent::saveMode;

            chooser->launchAsync(flag, [&buffer, sampleRate, bitDepth](const juce::FileChooser& fc)
            {
                juce::File outFile = fc.getResult();
                if (outFile == juce::File())
                    return;

                if (outFile.getFileExtension().isEmpty())
                    outFile = outFile.withFileExtension(".wav");

                // Save on background thread
                std::thread([outFile, &buffer, sampleRate, bitDepth]()
                {
                    juce::WavAudioFormat wavFormat;
                    std::unique_ptr<juce::FileOutputStream> stream(outFile.createOutputStream());

                    bool ok = false;
                    if (stream != nullptr)
                    {
                        std::unique_ptr<juce::AudioFormatWriter> writer(
                            wavFormat.createWriterFor(stream.get(), sampleRate, buffer.getNumChannels(), bitDepth, {}, 0));

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
                            juce::AlertWindow::showMessageBoxAsync(
                                juce::AlertWindow::InfoIcon,
                                "Saved",
                                "Processed file saved to:\n" + outFile.getFullPathName());
                        else
                            juce::AlertWindow::showMessageBoxAsync(
                                juce::AlertWindow::WarningIcon,
                                "Error",
                                "Failed to save file!");
                    });
                }).detach();
            });
        }

    private:
        FileIO() = default;
    };

} // namespace zazzDSP
