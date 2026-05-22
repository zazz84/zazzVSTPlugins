#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Setup buttons
    m_openSourceButton.setButtonText("Open Source");
    m_openSourceButton.onClick = [this] { openSourceButtonClicked(); };
    addAndMakeVisible(m_openSourceButton);

    m_analyzeButton.setButtonText("Analyze");
    m_analyzeButton.onClick = [this] { analyzeButtonClicked(); };
    addAndMakeVisible(m_analyzeButton);

    m_playButton.setButtonText("Play");
    m_playButton.onClick = [this] { playButtonClicked(); };
    addAndMakeVisible(m_playButton);

    m_saveOutputButton.setButtonText("Save Output");
    m_saveOutputButton.onClick = [this] { saveOutputButtonClicked(); };
    addAndMakeVisible(m_saveOutputButton);

    m_saveProjectButton.setButtonText("Save Project");
    m_saveProjectButton.onClick = [this] { saveProjectButtonClicked(); };
    addAndMakeVisible(m_saveProjectButton);

    m_loadProjectButton.setButtonText("Load Project");
    m_loadProjectButton.onClick = [this] { loadProjectButtonClicked(); };
    addAndMakeVisible(m_loadProjectButton);

    m_newProjectButton.setButtonText("New Project");
    m_newProjectButton.onClick = [this] { newProjectButtonClicked(); };
    addAndMakeVisible(m_newProjectButton);

    // Setup sliders
    m_minFrequencySlider.setRange(20.0, 20000.0, 1.0);
    m_minFrequencySlider.setValue(100.0);
    m_minFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    m_minFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(m_minFrequencySlider);

    m_maxFrequencySlider.setRange(20.0, 20000.0, 1.0);
    m_maxFrequencySlider.setValue(1000.0);
    m_maxFrequencySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    m_maxFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(m_maxFrequencySlider);

    m_padTimeSlider.setRange(0.0, 1000.0, 1.0);
    m_padTimeSlider.setValue(100.0);
    m_padTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    m_padTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(m_padTimeSlider);

    // Setup labels
    m_sourceFileNameLabel.setText("No file loaded", juce::dontSendNotification);
    addAndMakeVisible(m_sourceFileNameLabel);

    m_minFrequencyLabel.setText("Min Freq (Hz):", juce::dontSendNotification);
    addAndMakeVisible(m_minFrequencyLabel);

    m_maxFrequencyLabel.setText("Max Freq (Hz):", juce::dontSendNotification);
    addAndMakeVisible(m_maxFrequencyLabel);

    m_padTimeLabel.setText("Pad Time (ms):", juce::dontSendNotification);
    addAndMakeVisible(m_padTimeLabel);

    m_analysisResultLabel.setText("", juce::dontSendNotification);
    addAndMakeVisible(m_analysisResultLabel);

    // Setup spectrogram display
    addAndMakeVisible(m_spectrogramDisplay);

    // Setup audio format manager
    m_formatManager.registerBasicFormats();

    // Request audio channels
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
                                          [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }

    setSize(1200, 800);
}

//==============================================================================
MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_sampleRate = static_cast<int>(sampleRate);
    if (m_analyzer)
    {
        m_analyzer->setSampleRate(m_sampleRate);
    }
}

//==============================================================================
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (m_transportState != TransportState::Playing || m_bufferOutput.getNumSamples() == 0)
        return;

    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);

        const int numChannels = m_bufferOutput.getNumChannels();
        const int numSamples = bufferToFill.numSamples;
        const int totalOutputSamples = m_bufferOutput.getNumSamples();

        int outputIndex = 0;
        while (outputIndex < numSamples && m_playbackIndex < totalOutputSamples)
        {
            const int samplesToWrite = std::min(numSamples - outputIndex,
                                                 totalOutputSamples - m_playbackIndex);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* srcData = m_bufferOutput.getReadPointer(ch, m_playbackIndex);
                float* dstData = bufferToFill.buffer->getWritePointer(ch, outputIndex);
                std::copy(srcData, srcData + samplesToWrite, dstData);
            }

            outputIndex += samplesToWrite;
            m_playbackIndex += samplesToWrite;
        }

        // Stop playback when done
        if (m_playbackIndex >= totalOutputSamples)
        {
            m_transportState = TransportState::Stopped;
            m_playButton.setButtonText("Play");
            m_playbackIndex = 0;
        }
    }
}

//==============================================================================
void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

//==============================================================================
void MainComponent::resized()
{
    const int margin = 10;
    const int buttonHeight = 30;
    const int sliderHeight = 30;
    const int labelWidth = 120;
    const int rowHeight = buttonHeight + margin;
    const int waveformHeight = 150;

    int y = margin;

    // File operations
    m_openSourceButton.setBounds(margin, y, 120, buttonHeight);
    m_newProjectButton.setBounds(margin + 130, y, 120, buttonHeight);
    m_loadProjectButton.setBounds(margin + 260, y, 120, buttonHeight);
    y += rowHeight;

    m_sourceFileNameLabel.setBounds(margin, y, getWidth() - 2 * margin, 20);
    y += 25;

    // Spectrogram display
    m_spectrogramDisplay.setBounds(margin, y, getWidth() - 2 * margin, waveformHeight);
    y += waveformHeight + margin;

    // Filter parameters
    m_minFrequencyLabel.setBounds(margin, y, labelWidth, 20);
    m_minFrequencySlider.setBounds(margin + labelWidth, y, getWidth() - 2 * margin - labelWidth, sliderHeight);
    y += rowHeight;

    m_maxFrequencyLabel.setBounds(margin, y, labelWidth, 20);
    m_maxFrequencySlider.setBounds(margin + labelWidth, y, getWidth() - 2 * margin - labelWidth, sliderHeight);
    y += rowHeight;

    m_padTimeLabel.setBounds(margin, y, labelWidth, 20);
    m_padTimeSlider.setBounds(margin + labelWidth, y, getWidth() - 2 * margin - labelWidth, sliderHeight);
    y += rowHeight;

    // Analysis controls
    m_analyzeButton.setBounds(margin, y, 120, buttonHeight);
    m_playButton.setBounds(margin + 130, y, 120, buttonHeight);
    m_saveOutputButton.setBounds(margin + 260, y, 120, buttonHeight);
    m_saveProjectButton.setBounds(margin + 390, y, 120, buttonHeight);
    y += rowHeight;

    // Results
    m_analysisResultLabel.setBounds(margin, y, getWidth() - 2 * margin, 80);
}

//==============================================================================
void MainComponent::openSourceButtonClicked()
{
    if (!m_wavOpenChooser)
    {
        m_wavOpenChooser = std::make_unique<juce::FileChooser>(
            "Select a WAV file...",
            juce::File{},
            "*.wav");
    }

    m_wavOpenChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File{})
                return;

            m_sourceFilePath = file.getFullPathName();
            auto* reader = m_formatManager.createReaderFor(file);

            if (reader != nullptr)
            {
                const int samples = (int)reader->lengthInSamples;
                if (samples != 0)
                {
                    m_sampleRate = static_cast<int>(reader->sampleRate);
                    m_fileName = file.getFileName();

                    {
                        std::lock_guard<std::mutex> lock(m_bufferMutex);
                        m_bufferSource.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                        reader->read(&m_bufferSource, 0, (int)reader->lengthInSamples, 0, true, true);
                    }

                    m_sourceFileNameLabel.setText(m_fileName, juce::dontSendNotification);

                    // Update analyzer
                    if (!m_analyzer)
                    {
                        m_analyzer = std::make_unique<DominantFrequencyAnalyzer>(m_sampleRate, 4096);
                    }
                    else
                    {
                        m_analyzer->setSampleRate(m_sampleRate);
                    }

                    // Display spectrogram
                    m_spectrogramDisplay.setAudioBuffer(m_bufferSource);
                    m_spectrogramDisplay.setSampleRate(m_sampleRate);

                    repaint();
                }
                delete reader;
            }
        });
}

//==============================================================================
void MainComponent::analyzeButtonClicked()
{
    if (m_bufferSource.getNumSamples() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "No source file loaded!");
        return;
    }

    if (!m_analyzer)
    {
        m_analyzer = std::make_unique<DominantFrequencyAnalyzer>(m_sampleRate, 4096);
    }

    const float minFreq = static_cast<float>(m_minFrequencySlider.getValue());
    const float maxFreq = static_cast<float>(m_maxFrequencySlider.getValue());
    const int padSamples = static_cast<int>(m_padTimeSlider.getValue() * m_sampleRate / 1000.0);

    if (minFreq >= maxFreq)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "Minimum frequency must be less than maximum frequency!");
        return;
    }

    // Find matching segments
    m_analyzer->findSegments(m_bufferSource, minFreq, maxFreq, padSamples, m_foundSegments);

    // Extract to output buffer
    if (!m_foundSegments.empty())
    {
        m_analyzer->extractSegmentsToBuffer(m_bufferSource, m_foundSegments, m_bufferOutput);

        juce::String infoText = "Found " + juce::String(static_cast<int>(m_foundSegments.size())) +
                                " segments\nTotal duration: " +
                                juce::String(m_bufferOutput.getNumSamples() / static_cast<double>(m_sampleRate), 2) +
                                " seconds";
        m_analysisResultLabel.setText(infoText, juce::dontSendNotification);

        m_playbackIndex = 0;
        m_transportState = TransportState::Stopped;
        m_playButton.setButtonText("Play");
    }
    else
    {
        m_analysisResultLabel.setText("No matching segments found", juce::dontSendNotification);
        m_bufferOutput.setSize(0, 0);
    }

    repaint();
}

//==============================================================================
void MainComponent::playButtonClicked()
{
    if (m_foundSegments.empty())
        return;

    if (m_transportState == TransportState::Stopped)
    {
        if (m_bufferOutput.getNumSamples() > 0)
        {
            m_transportState = TransportState::Playing;
            m_playButton.setButtonText("Stop");
            m_playbackIndex = 0;
        }
    }
    else
    {
        m_transportState = TransportState::Stopped;
        m_playButton.setButtonText("Play");
        m_playbackIndex = 0;
    }
}

//==============================================================================
void MainComponent::saveOutputButtonClicked()
{
    if (m_bufferOutput.getNumSamples() == 0)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "No output to save!");
        return;
    }

    if (!m_wavSaveChooser)
    {
        m_wavSaveChooser = std::make_unique<juce::FileChooser>(
            "Save output WAV file...",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.wav");
    }

    m_wavSaveChooser->launchAsync(
        juce::FileBrowserComponent::saveMode,
        [this](const juce::FileChooser& fc)
        {
            juce::File outFile = fc.getResult();
            if (outFile == juce::File{})
                return;

            if (auto stream = outFile.createOutputStream())
            {
                juce::WavAudioFormat wavFormat;
                if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(
                    wavFormat.createWriterFor(stream.get(), m_sampleRate, m_bufferOutput.getNumChannels(), 32, {}, 0)))
                {
                    writer->writeFromAudioSampleBuffer(m_bufferOutput, 0, m_bufferOutput.getNumSamples());
                    writer.reset();
                    stream.release();

                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::InfoIcon,
                        "Success",
                        "Output saved to:\n" + outFile.getFullPathName());
                }
            }
        });
}

//==============================================================================
void MainComponent::saveProjectButtonClicked()
{
    if (!m_projectSaveChooser)
    {
        m_projectSaveChooser = std::make_unique<juce::FileChooser>(
            "Save Analysis Project",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.json");
    }

    m_projectSaveChooser->launchAsync(
        juce::FileBrowserComponent::saveMode,
        [this](const juce::FileChooser& fc)
        {
            juce::File projectFile = fc.getResult();
            if (projectFile == juce::File())
                return;

            saveProjectToFile(projectFile);
        });
}

//==============================================================================
void MainComponent::loadProjectButtonClicked()
{
    if (!m_projectLoadChooser)
    {
        m_projectLoadChooser = std::make_unique<juce::FileChooser>(
            "Load Analysis Project",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.json");
    }

    m_projectLoadChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            juce::File projectFile = fc.getResult();
            if (projectFile == juce::File())
                return;

            loadProjectFromFile(projectFile);
        });
}

//==============================================================================
void MainComponent::newProjectButtonClicked()
{
    {
        std::lock_guard<std::mutex> lock(m_bufferMutex);
        m_bufferSource.setSize(0, 0);
        m_bufferOutput.setSize(0, 0);
    }
    m_foundSegments.clear();
    m_fileName = "";
    m_sourceFilePath = "";
    m_sourceFileNameLabel.setText("", juce::dontSendNotification);
    m_analysisResultLabel.setText("", juce::dontSendNotification);

    m_playbackIndex = 0;
    m_transportState = TransportState::Stopped;
    m_playButton.setButtonText("Play");

    m_minFrequencySlider.setValue(100.0);
    m_maxFrequencySlider.setValue(1000.0);
    m_padTimeSlider.setValue(100.0);

    // Clear spectrogram display
    m_spectrogramDisplay.setAudioBuffer(juce::AudioBuffer<float>());

    repaint();
}

//==============================================================================
void MainComponent::saveProjectToFile(const juce::File& projectFile)
{
    auto projectObject = std::make_unique<juce::DynamicObject>();

    projectObject->setProperty("sourceFilePath", m_sourceFilePath);
    projectObject->setProperty("minFrequency", m_minFrequencySlider.getValue());
    projectObject->setProperty("maxFrequency", m_maxFrequencySlider.getValue());
    projectObject->setProperty("padTimeMs", m_padTimeSlider.getValue());
    projectObject->setProperty("sampleRate", m_sampleRate);

    // Serialize segments
    juce::Array<juce::var> segmentsArray;
    for (const auto& segment : m_foundSegments)
    {
        auto segObj = std::make_unique<juce::DynamicObject>();
        segObj->setProperty("startSample", segment.startSample);
        segObj->setProperty("endSample", segment.endSample);
        segObj->setProperty("dominantFrequency", segment.dominantFrequency);
        segObj->setProperty("magnitude", segment.magnitude);
        segmentsArray.add(juce::var(segObj.release()));
    }
    projectObject->setProperty("segments", segmentsArray);

    juce::var jsonVar(projectObject.release());
    projectFile.replaceWithText(juce::JSON::toString(jsonVar, true));

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Success",
        "Project saved to:\n" + projectFile.getFullPathName());
}

//==============================================================================
void MainComponent::loadProjectFromFile(const juce::File& projectFile)
{
    juce::String fileContents = projectFile.loadFileAsString();
    auto jsonVar = juce::JSON::parse(fileContents);

    if (!jsonVar.isObject())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Error",
            "Invalid project file!");
        return;
    }

    auto* obj = jsonVar.getDynamicObject();

    // Load source file
    if (obj->hasProperty("sourceFilePath"))
    {
        juce::String filePath = obj->getProperty("sourceFilePath").toString();
        juce::File sourceFile(filePath);

        if (sourceFile.exists())
        {
            auto* reader = m_formatManager.createReaderFor(sourceFile);
            if (reader != nullptr)
            {
                m_sampleRate = static_cast<int>(reader->sampleRate);
                m_fileName = sourceFile.getFileName();
                m_sourceFilePath = sourceFile.getFullPathName();

                {
                    std::lock_guard<std::mutex> lock(m_bufferMutex);
                    m_bufferSource.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                    reader->read(&m_bufferSource, 0, (int)reader->lengthInSamples, 0, true, true);
                }

                m_sourceFileNameLabel.setText(m_fileName, juce::dontSendNotification);

                // Display spectrogram
                m_spectrogramDisplay.setAudioBuffer(m_bufferSource);
                m_spectrogramDisplay.setSampleRate(m_sampleRate);

                delete reader;
            }
        }
    }

    // Load parameters
    if (obj->hasProperty("minFrequency"))
        m_minFrequencySlider.setValue(obj->getProperty("minFrequency"));
    if (obj->hasProperty("maxFrequency"))
        m_maxFrequencySlider.setValue(obj->getProperty("maxFrequency"));
    if (obj->hasProperty("padTimeMs"))
        m_padTimeSlider.setValue(obj->getProperty("padTimeMs"));

    // Load segments
    if (obj->hasProperty("segments"))
    {
        m_foundSegments.clear();
        const juce::var& segmentsVar = obj->getProperty("segments");
        if (segmentsVar.isArray())
        {
            const juce::Array<juce::var>* segArray = segmentsVar.getArray();
            for (const auto& segVar : *segArray)
            {
                if (segVar.isObject())
                {
                    auto* segObj = segVar.getDynamicObject();
                    DominantFrequencyAnalyzer::AnalysisSegment seg;
                    seg.startSample = (int)segObj->getProperty("startSample");
                    seg.endSample = (int)segObj->getProperty("endSample");
                    seg.dominantFrequency = (float)segObj->getProperty("dominantFrequency");
                    seg.magnitude = (float)segObj->getProperty("magnitude");
                    m_foundSegments.push_back(seg);
                }
            }

            // Recreate output buffer from segments
            if (!m_foundSegments.empty() && m_bufferSource.getNumSamples() > 0)
            {
                if (!m_analyzer)
                {
                    m_analyzer = std::make_unique<DominantFrequencyAnalyzer>(m_sampleRate, 4096);
                }
                m_analyzer->extractSegmentsToBuffer(m_bufferSource, m_foundSegments, m_bufferOutput);

                juce::String infoText = "Loaded " + juce::String(static_cast<int>(m_foundSegments.size())) +
                                        " segments\nTotal duration: " +
                                        juce::String(m_bufferOutput.getNumSamples() / static_cast<double>(m_sampleRate), 2) +
                                        " seconds";
                m_analysisResultLabel.setText(infoText, juce::dontSendNotification);
            }
        }
    }

    repaint();
}
