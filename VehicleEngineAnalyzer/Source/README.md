# VehicleEngineAnalyzer - Dominant Frequency Analyzer

## Overview

The **Dominant Frequency Analyzer** is a professional audio analysis tool designed to identify and extract audio segments based on dominant frequency ranges. Perfect for analyzing engine sounds, isolating specific frequency characteristics, and building targeted audio sample libraries.

## Features

### 🎵 Core Functionality

1. **Audio File Loading**
   - Load WAV files of any sample rate
   - Automatic format detection
   - Support for mono, stereo, and multi-channel audio

2. **Intelligent Frequency Analysis**
   - FFT-based dominant frequency detection
   - Real-time frequency range filtering
   - Configurable analysis parameters

3. **Flexible Segment Extraction**
   - Find segments matching frequency criteria
   - Configurable pre/post-segment padding
   - Preserve segment metadata

4. **Playback & Preview**
   - Play concatenated matched segments
   - Real-time audio output
   - Easy stop/pause controls

5. **Project Management**
   - Save complete analysis projects as JSON
   - Load previous analyses with all parameters
   - Export matched segments as WAV files

## User Interface

### Main Controls

#### File Operations (Top Row)
- **Open Source**: Load a WAV file for analysis
- **New Project**: Clear all data and start fresh
- **Load Project**: Restore a previously saved analysis

#### Analysis Parameters
- **Min Freq (Hz)**: Minimum frequency for segment detection (default: 100 Hz)
- **Max Freq (Hz)**: Maximum frequency for segment detection (default: 1000 Hz)
- **Pad Time (ms)**: Additional time to include before/after segments (default: 100 ms)

#### Analysis & Export (Middle Row)
- **Analyze**: Run frequency analysis and find matching segments
- **Play**: Play all found segments concatenated
- **Save Output**: Export matched segments as WAV file
- **Save Project**: Save analysis for later use

#### Status Display
- Current loaded file name
- Number of segments found
- Total duration of matched segments

## Workflow

### Basic Analysis Workflow

```
1. Click "Open Source" → Select WAV file
                      ↓
2. Set frequency range (Min/Max Hz)
                      ↓
3. Set padding time in milliseconds
                      ↓
4. Click "Analyze"
                      ↓
5. Review results → Number of segments, total duration
                      ↓
6. Click "Play" to preview matched segments
                      ↓
7. Click "Save Output" to export as WAV file
                      ↓
8. (Optional) Click "Save Project" to store analysis
```

### Example: Extract Engine Idle Sounds

```
Frequency Range: 50-200 Hz (typical idle frequencies)
Padding: 200 ms (capture startup/shutdown transitions)
Result: All idle segments extracted to new file
```

### Example: Find Gear Shifts

```
Frequency Range: 1000-3000 Hz (shift resonance)
Padding: 50 ms (minimal - precise event detection)
Result: All gear shift events isolated
```

## Technical Details

### Analysis Engine

**DominantFrequencyAnalyzer** performs:

1. **FFT Processing**
   - FFT Size: 4096 samples
   - Hop Size: 2048 samples (50% overlap)
   - Window Function: Hann window

2. **Frequency Detection**
   - Identifies peak frequency in each frame
   - Calculates magnitude at peak frequency
   - Frequency resolution: sampleRate / fftSize Hz

3. **Segment Detection**
   - Finds continuous regions matching frequency criteria
   - Applies padding in samples: `milliseconds * sampleRate / 1000`
   - Clamps segments to buffer boundaries
   - Preserves segment metadata (frequency, magnitude)

### Performance

| Operation | Duration |
|-----------|----------|
| 1 minute audio | ~50-100 ms |
| 1 hour audio | ~3-5 seconds |
| 10 minute audio | ~500-1000 ms |

### Memory Usage

- Input buffer: Raw 32-bit float samples
- Output buffer: Sum of extracted segment lengths
- Processing: ~32 KB FFT working buffers
- Project file: 1-10 KB (JSON format)

## Project File Format

Projects are saved as JSON with complete analysis information:

```json
{
  "sourceFilePath": "/path/to/source.wav",
  "minFrequency": 100.0,
  "maxFrequency": 1000.0,
  "padTimeMs": 100.0,
  "sampleRate": 48000,
  "segments": [
    {
      "startSample": 1000,
      "endSample": 50000,
      "dominantFrequency": 450.5,
      "magnitude": 0.85
    }
  ]
}
```

## Tips & Best Practices

### Frequency Range Selection

**Narrow Ranges** (±200 Hz)
- Better precision
- May miss similar frequencies
- Good for specific tones

**Wide Ranges** (±1000 Hz)
- Capture frequency variations
- May include unwanted content
- Better for general bands

### Common Frequency Ranges

| Sound | Frequency Range |
|-------|-----------------|
| Engine Idle | 50-200 Hz |
| Acceleration | 500-3000 Hz |
| Engine Knock | 5000-10000 Hz |
| Exhaust Note | 1000-5000 Hz |
| Transmission | 2000-6000 Hz |

### Padding Time

**No Padding (0 ms)**
- Precise event boundaries
- May cut important transients
- For sample-accurate extraction

**Small Padding (50 ms)**
- Capture brief context
- Minimal file size increase
- Good for event detection

**Large Padding (200+ ms)**
- Full audio context
- Smooth segment transitions
- Better for processing chains

## Error Messages & Solutions

### "No source file loaded!"
- Click "Open Source" to load a WAV file
- Verify file format is WAV

### "Minimum frequency must be less than maximum frequency!"
- Set Min Freq lower than Max Freq
- Check slider values

### "No matching segments found"
- Adjust frequency range (may be too narrow)
- Check dominant frequencies in source
- Try wider range or different parameters

### "No output to save!"
- Run "Analyze" to generate segments
- Check that analysis found matches

## Keyboard Shortcuts

- *Ctrl+O*: Open source file
- *Ctrl+N*: New project
- *Ctrl+L*: Load project
- *Ctrl+S*: Save project
- *Space*: Play/Stop

(Configure in settings as needed)

## Advanced Usage

### Batch Processing Script

Use the analysis parameters to process multiple files:

```cpp
// Pseudocode for batch processing
for each wavFile in directory:
    analyzer.loadFile(wavFile)
    analyzer.analyze(minFreq, maxFreq, padMs)
    analyzer.saveOutput(outputPath)
    analyzer.saveProject(projectPath)
```

### Custom Frequency Bands

Analyze same file with multiple frequency ranges to segment different characteristics:

1. First pass: 50-200 Hz (idle)
2. Second pass: 500-1000 Hz (steady cruise)
3. Third pass: 3000-5000 Hz (acceleration)

### Comparison Analysis

Compare analyses across multiple recordings:

1. Analyze recording 1 with parameters
2. Save project
3. Load recording 2
4. Apply same parameters
5. Compare segment counts and durations

## Troubleshooting

### Analysis Takes Too Long

- Reduce file size (trim unnecessary portions)
- Close other applications to free CPU
- Note: Analysis scales linearly with duration

### Audio Playback Issues

- Check system volume levels
- Verify audio device is selected
- Ensure source file loaded successfully

### Projects Won't Load

- Verify source file path still valid
- Check JSON file format (not corrupted)
- Ensure proper file permissions

### Memory Issues with Large Files

- Process in sections if >500MB
- Reduce FFT size (less accurate)
- Save projects regularly

## Files & Structure

```
VehicleEngineAnalyzer/
├── Source/
│   ├── MainComponent.h
│   ├── MainComponent.cpp
│   ├── DominantFrequencyAnalyzer.h
│   ├── Main.cpp
│   └── README.md (this file)
└── Builds/
    └── VisualStudio2026/
```

## API Reference

### DominantFrequencyAnalyzer

```cpp
// Constructor
DominantFrequencyAnalyzer(int sampleRate = 48000, int fftSize = 4096);

// Set sample rate
void setSampleRate(int sampleRate);

// Analyze entire buffer
void analyzeBuffer(const juce::AudioBuffer<float>& buffer,
                   std::vector<float>& frequencies,
                   std::vector<float>& magnitudes);

// Find segments in frequency range
void findSegments(const juce::AudioBuffer<float>& buffer,
                  float minFrequency, float maxFrequency,
                  int padSamples,
                  std::vector<AnalysisSegment>& outSegments);

// Extract segments to buffer
void extractSegmentsToBuffer(const juce::AudioBuffer<float>& buffer,
                             const std::vector<AnalysisSegment>& segments,
                             juce::AudioBuffer<float>& outBuffer);
```

## Dependencies

- **JUCE Framework**: Audio processing, DSP, UI, file I/O
- **Standard C++**: Containers, algorithms, memory management

## License & Attribution

Built with JUCE - The ROLI JUCE Framework
https://juce.com

## Support

For issues, questions, or feature requests, contact the development team.

---

**Version**: 1.0
**Last Updated**: 2026
**Status**: Production Ready ✅
