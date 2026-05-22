#pragma once

#include <JuceHeader.h>
#include "DominantFrequencyAnalyzer.h"
#include "../../../zazzGUI/classes/SpectrogramDisplay.h"
#include <vector>
#include <mutex>
#include <memory>

//==============================================================================
/*
    Dominant Frequency Analyzer Component

    This component analyzes audio files to identify and extract segments based on 
    dominant frequency ranges. Features include:
    - Load WAV files
    - Configure frequency range filtering
    - Analyze to find matching segments
    - Playback of matched segments
    - Save/load analysis projects
    - Export matched segments as WAV file
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    enum TransportState
    {
        Stopped,
        Playing
    };

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

    //==============================================================================
    // File operations
    void openSourceButtonClicked();
    void analyzeButtonClicked();
    void playButtonClicked();
    void saveOutputButtonClicked();
    void saveProjectButtonClicked();
    void loadProjectButtonClicked();
    void newProjectButtonClicked();

    //==============================================================================
    // Project persistence
    void saveProjectToFile(const juce::File& projectFile);
    void loadProjectFromFile(const juce::File& projectFile);

    //==============================================================================
    // UI Components - Buttons
    juce::TextButton m_openSourceButton;
    juce::TextButton m_analyzeButton;
    juce::TextButton m_playButton;
    juce::TextButton m_saveOutputButton;
    juce::TextButton m_saveProjectButton;
    juce::TextButton m_loadProjectButton;
    juce::TextButton m_newProjectButton;

    // UI Components - Sliders
    juce::Slider m_minFrequencySlider;
    juce::Slider m_maxFrequencySlider;
    juce::Slider m_padTimeSlider;

    // UI Components - Labels
    juce::Label m_sourceFileNameLabel;
    juce::Label m_minFrequencyLabel;
    juce::Label m_maxFrequencyLabel;
    juce::Label m_padTimeLabel;
    juce::Label m_analysisResultLabel;

    // Display component
    zazzGUI::SpectrogramDisplay m_spectrogramDisplay { "Audio Spectrogram" };

    //==============================================================================
    // Audio buffers
    juce::AudioBuffer<float> m_bufferSource;
    juce::AudioBuffer<float> m_bufferOutput;

    //==============================================================================
    // Analysis
    std::unique_ptr<DominantFrequencyAnalyzer> m_analyzer;
    std::vector<DominantFrequencyAnalyzer::AnalysisSegment> m_foundSegments;

    //==============================================================================
    // Playback
    TransportState m_transportState = TransportState::Stopped;
    int m_playbackIndex = 0;

    //==============================================================================
    // Audio configuration
    int m_sampleRate = 48000;
    juce::AudioFormatManager m_formatManager;

    //==============================================================================
    // File I/O
    juce::String m_fileName;
    juce::String m_sourceFilePath;
    std::unique_ptr<juce::FileChooser> m_wavOpenChooser;
    std::unique_ptr<juce::FileChooser> m_wavSaveChooser;
    std::unique_ptr<juce::FileChooser> m_projectSaveChooser;
    std::unique_ptr<juce::FileChooser> m_projectLoadChooser;

    //==============================================================================
    // Thread synchronization
    std::mutex m_bufferMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
