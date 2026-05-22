# VehicleEngineAnalyzer - Quick Start Guide

## 🚀 Getting Started in 5 Minutes

### Step 1: Build the Project
```
1. Open VehicleEngineAnalyzer.sln in Visual Studio 2026
2. Build → Build Solution
3. Wait for successful build ✅
```

### Step 2: Run the Application
```
1. Debug → Start Debugging (F5)
2. Application window opens
3. Ready to analyze!
```

### Step 3: Load an Audio File
```
1. Click "Open Source" button
2. Select a WAV file
3. Filename appears in status area
4. File is loaded and ready
```

### Step 4: Analyze
```
1. Set Min Frequency: 100 Hz (default)
2. Set Max Frequency: 1000 Hz (default)
3. Set Pad Time: 100 ms (default)
4. Click "Analyze"
5. Results show: Number of segments + Duration
```

### Step 5: Preview or Export
```
PREVIEW: Click "Play" to hear segments
EXPORT: Click "Save Output" to save as WAV
PROJECT: Click "Save Project" to store analysis
```

---

## 📋 Quick Reference

### Common Tasks

| Task | Steps |
|------|-------|
| Load file | Click "Open Source" → Select WAV |
| Find engine idle | Min: 50, Max: 200, Analyze |
| Find gear shifts | Min: 1000, Max: 3000, Analyze |
| Extract acceleration | Min: 500, Max: 2000, Analyze |
| Export results | Click "Save Output" → Choose location |
| Save for later | Click "Save Project" → Choose location |
| Start fresh | Click "New Project" |

### Common Frequency Ranges

| Sound | Min | Max |
|-------|-----|-----|
| Idle | 50 | 200 |
| Acceleration | 500 | 3000 |
| Cruise | 1000 | 2000 |
| High RPM | 3000 | 5000 |
| Knock | 5000 | 10000 |

---

## 🎯 Example Workflows

### Workflow 1: Extract Engine Sounds (5 minutes)
```
1. Open Source → select engine_recording.wav
2. Min: 50, Max: 200, Pad: 300 ms
3. Analyze → Get idle segments
4. Save Output → engine_idle.wav
5. Done! ✅
```

### Workflow 2: Find All Events (10 minutes)
```
1. Open Source → select recording.wav

   PASS 1 - Idle
   2. Min: 50, Max: 200, Analyze
   3. Save Output → idle.wav

   PASS 2 - Acceleration  
   4. Min: 500, Max: 2000, Analyze
   5. Save Output → accel.wav

   PASS 3 - High RPM
   6. Min: 3000, Max: 5000, Analyze
   7. Save Output → high_rpm.wav

8. Done! All events extracted ✅
```

### Workflow 3: Compare Recordings (5 minutes)
```
1. Open Source → recording1.wav
2. Set parameters: 100-1000 Hz, 100ms pad
3. Analyze → Note results
4. Save Project → recording1.json

5. Open Source → recording2.wav
6. Use SAME parameters
7. Analyze → Compare results
8. Observations noted ✅
```

---

## 🔧 Troubleshooting

### "No source file loaded!"
→ Click "Open Source" and select a WAV file

### "No matching segments found"
→ Your frequency range might be wrong
→ Try a wider range: increase Min, decrease Max
→ Or try: Min: 20, Max: 20000 (entire spectrum)

### "Analyze button does nothing"
→ File not loaded? See first issue
→ Min frequency ≥ Max frequency? Adjust sliders

### "Playback has no sound"
→ Check system volume
→ Check audio device in system settings
→ Try saving output first to verify audio exists

### "Save fails"
→ Check folder permissions
→ Ensure path exists
→ Try different location

---

## 💡 Tips & Tricks

### Tip 1: Start Broad, Then Narrow
```
First try: 20-20000 Hz (everything)
See what frequency matches are found
Then narrow range based on results
```

### Tip 2: Use Padding Wisely
```
Minimal (0-50 ms): Precise event detection
Medium (100-300 ms): Capture context
Large (300+ ms): Full audio environment
```

### Tip 3: Save Projects Often
```
After each successful analysis
At different parameter settings
Creates analysis history
Easy to reproduce
```

### Tip 4: Multi-Pass Strategy
```
Analyze same file multiple times
Different frequency ranges
Extract different characteristics
Build complete audio library
```

### Tip 5: Batch Processing
```
Use same parameters for multiple files
Compare results across recordings
Identify variations
Validate consistency
```

---

## 📊 What to Expect

### Analysis Results Display
```
✓ Found 25 segments
✓ Total duration: 45.3 seconds
✓ Sample frequency range: 50-5000 Hz
✓ Ready to play or export
```

### Export File Details
```
Format: 32-bit float WAV
Sample Rate: (matches source)
Channels: (matches source)
Content: All matching segments concatenated
Duration: Sum of all segment durations
```

### Project File Contents
```
JSON format (human readable)
All analysis parameters
List of found segments
Start/end times for each
Dominant frequency info
Save timestamp
```

---

## ⏱️ Performance Expectations

| File Size | Analysis Time |
|-----------|---------------|
| 1 minute | ~50-100 ms |
| 5 minutes | ~250-500 ms |
| 10 minutes | ~500-1000 ms |
| 1 hour | ~3-5 seconds |

**Note**: Times are approximate and depend on CPU speed

---

## 📁 File Locations

### Source Files
```
VehicleEngineAnalyzer/Source/
├── DominantFrequencyAnalyzer.h
├── MainComponent.h
├── MainComponent.cpp
├── Main.cpp
└── README.md
```

### Output Locations (Default)
```
WAV Files: Documents/
Projects: Documents/
```

**Pro Tip**: Create a dedicated folder for analysis projects
```
D:\Audio Analysis\Projects\
D:\Audio Analysis\Exports\
```

---

## 🎓 Learning Resources

### In Application
- Hover over controls for tooltips
- Check status label for results
- Error messages are descriptive

### Documentation
- **README.md** - Complete user guide
- **AnalyzerExamples.cpp** - Code examples
- **IMPLEMENTATION_SUMMARY.md** - Technical details

### Online Help
- See README.md for API reference
- See AnalyzerExamples.cpp for workflows

---

## ⚡ Keyboard Shortcuts (Future)

Currently use mouse clicks for all operations.
Keyboard shortcuts can be added in future updates:
```
Ctrl+O - Open file
Ctrl+N - New project
Ctrl+S - Save project
Ctrl+L - Load project
Space  - Play/Stop
```

---

## 🐛 Common Questions

**Q: Can I analyze non-WAV files?**
A: Currently WAV only. MP3/FLAC support could be added.

**Q: How long can audio files be?**
A: Limited by RAM. Typically: 100+ MB files work fine.

**Q: Can I edit segments after analysis?**
A: Not in current version. Edit frequency range and re-analyze.

**Q: How accurate is the frequency detection?**
A: Frequency resolution = SampleRate / 4096 Hz (high precision)

**Q: Can I process in real-time?**
A: Current version requires full file loading first.

**Q: Is there batch processing?**
A: Not automated. Can manually process multiple files.

---

## 🎉 You're Ready!

1. ✅ Application installed and running
2. ✅ Understand basic workflow
3. ✅ Know common frequency ranges
4. ✅ Can troubleshoot common issues
5. ✅ Ready to analyze!

---

## 📞 Next Steps

1. **Try a test file**
   - Load any WAV file
   - Use default parameters
   - See what segments are found

2. **Explore ranges**
   - Try different frequency ranges
   - Note what gets detected
   - Learn your audio characteristics

3. **Save and compare**
   - Save a project
   - Try different parameters
   - Load and compare results

4. **Build your library**
   - Analyze multiple recordings
   - Extract different frequency bands
   - Create organized audio library

---

## 📈 Advanced (When Ready)

- Custom FFT sizes (code modification)
- Additional frequency bands (add passes)
- Crossfading between segments
- Envelope detection
- Pitch tracking
- Phase analysis

See IMPLEMENTATION_SUMMARY.md for roadmap.

---

**Version**: 1.0  
**Last Updated**: 2026  
**Status**: Ready to Use ✅  

**Enjoy analyzing! 🎵**
