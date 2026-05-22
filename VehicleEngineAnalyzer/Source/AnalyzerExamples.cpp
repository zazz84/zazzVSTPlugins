/*
	==========================================================================
	DOMINANT FREQUENCY ANALYZER - USAGE EXAMPLES
	==========================================================================
	
	This file contains practical examples for using the Analyzer component.
	
	==========================================================================
	EXAMPLE 1: BASIC USAGE
	==========================================================================
*/

// Start with default settings
void basicUsageExample()
{
	// The analyzer initializes automatically with:
	// - Min Frequency: 100 Hz
	// - Max Frequency: 1000 Hz
	// - Pad Time: 100 ms

	// 1. Load a file by clicking "Open Source"
	//    -> File is loaded to m_bufferSource
	//    -> Sample rate is detected
	//    -> Analyzer is initialized

	// 2. Adjust parameters if needed
	//    m_minFrequencySlider.setValue(50.0);
	//    m_maxFrequencySlider.setValue(500.0);
	//    m_padTimeSlider.setValue(200.0);

	// 3. Click "Analyze"
	//    -> Finds all segments matching frequency range
	//    -> Displays results (number of segments, duration)

	// 4. Click "Play" to preview
	//    -> Plays all concatenated segments through audio output

	// 5. Click "Save Output" to export
	//    -> Saves matched segments as WAV file
}

/*
	==========================================================================
	EXAMPLE 2: ENGINE SOUND ANALYSIS - IDLE DETECTION
	==========================================================================
*/

void analyzeEngineIdle()
{
	// Engine idle typically ranges from 50-200 Hz
	// We want to capture the start and end of idle with context

	// Settings:
	// - Min Frequency: 50 Hz (sub-harmonic of 4-cyl engine)
	// - Max Frequency: 200 Hz (includes harmonics)
	// - Pad Time: 300 ms (capture throttle transitions)

	// Typical results:
	// - Find all idle segments in recording
	// - Each segment includes throttle-up/down transients
	// - Total extracted duration = sum of all idle segments
	// - Can export as separate file for audio library

	// Workflow:
	// 1. Load engine recording containing idle, acceleration, cruise
	// 2. Set Min=50, Max=200, Pad=300
	// 3. Analyze to extract all idle portions
	// 4. Save as "engine_idle_samples.wav"
	// 5. Save project for reproducibility
}

/*
	==========================================================================
	EXAMPLE 3: GEAR SHIFT DETECTION
	==========================================================================
*/

void analyzeGearShifts()
{
	// Gear shifts create characteristic frequency peaks
	// around 1-3 kHz due to drivetrain resonance

	// Settings:
	// - Min Frequency: 1000 Hz
	// - Max Frequency: 3000 Hz
	// - Pad Time: 50 ms (minimal, precise event detection)

	// Results:
	// - Each segment = one gear shift event
	// - Minimal padding preserves event boundaries
	// - Can analyze shift characteristics
	// - Export for gear shift SFX library

	// Workflow:
	// 1. Load highway driving recording
	// 2. Set Min=1000, Max=3000, Pad=50
	// 3. Analyze to find all shift events
	// 4. Check results (typical: 20-50 shifts in 1 hour)
	// 5. Save output file with extracted shifts
	// 6. Can further segment by shift speed/load
}

/*
	==========================================================================
	EXAMPLE 4: MULTI-PASS ANALYSIS
	==========================================================================
*/

void multiPassAnalysis()
{
	// Analyze same file multiple times with different parameters
	// to segment different frequency characteristics

	// Pass 1: Extract idle segments
	// Min: 50, Max: 200, Pad: 300ms
	// Save: "idle_samples.wav"

	// Pass 2: Extract acceleration
	// Min: 500, Max: 1500, Pad: 100ms
	// Save: "acceleration_samples.wav"

	// Pass 3: Extract high RPM cruise
	// Min: 2000, Max: 4000, Pad: 100ms
	// Save: "cruise_samples.wav"

	// Pass 4: Extract knocking/detonation
	// Min: 5000, Max: 10000, Pad: 50ms
	// Save: "detonation_samples.wav"

	// Benefit: Complete audio library from single recording
	// Each category extracted with appropriate parameters
}

/*
	==========================================================================
	EXAMPLE 5: COMPARISON ANALYSIS
	==========================================================================
*/

void compareEngineConditions()
{
	// Compare engine sounds in different conditions
	// by analyzing with identical parameters

	// Recording 1: Cold engine start
	// -> Analyze with same parameters
	// -> Note idle frequency, segments found

	// Recording 2: Warm engine
	// -> Same analysis parameters
	// -> Compare results

	// Recording 3: Engine with issue
	// -> Same parameters
	// -> Identify frequency shifts or changes

	// Results:
	// - Differences in frequency ranges indicate engine state
	// - Number of segments reflects variability
	// - Duration changes show power output variations
}

/*
	==========================================================================
	EXAMPLE 6: TROUBLESHOOTING - NO SEGMENTS FOUND
	==========================================================================
*/

void troubleshootNoSegments()
{
	// If "No matching segments found" after analysis:

	// Step 1: Check frequency range
	// - Typical engine sounds: 50-5000 Hz
	// - If using 1000-2000 Hz and finding nothing,
	//   try 500-3000 Hz (wider range)

	// Step 2: Verify file has audio
	// - Check that file loaded successfully
	// - Filename displayed in UI
	// - Try with known-good file

	// Step 3: Adjust padding
	// - Padding won't prevent segment detection
	// - But very small segments might be missed
	// - Increase Min frequency to merge nearby events

	// Step 4: Check dominant frequencies
	// - Is the actual audio in your range?
	// - Use multiple passes to find content:
	//   * First pass: 20-100 Hz (sub-bass)
	//   * Second pass: 100-500 Hz (bass)
	//   * Third pass: 500-2000 Hz (mid)
	//   * Fourth pass: 2000-10000 Hz (high)
}

/*
	==========================================================================
	EXAMPLE 7: PROJECT MANAGEMENT WORKFLOW
	==========================================================================
*/

void projectManagementExample()
{
	// Save project at key points:

	// After initial analysis:
	// -> Click "Save Project"
	// -> File: "engine_analysis_v1.json"
	// -> Contains all found segments

	// After refining parameters:
	// -> Click "Save Project"
	// -> File: "engine_analysis_refined.json"
	// -> Compare results with v1

	// Load for batch processing:
	// -> Load "engine_analysis_v1.json"
	// -> All previous segments restored
	// -> Can export again with different settings
	// -> Or continue analysis with modifications

	// Benefits:
	// - Reproducible analysis
	// - Easy parameter comparison
	// - Archive of analysis history
	// - Share analysis with team
}

/*
	==========================================================================
	EXAMPLE 8: EXPORT & INTEGRATION
	==========================================================================
*/

void exportAndIntegration()
{
	// After analysis, several options:

	// Option 1: Export concatenated segments
	// -> Click "Save Output"
	// -> Single WAV file containing all matched segments
	// -> Useful for: SFX libraries, sample packs

	// Option 2: Save project for future use
	// -> Click "Save Project"
	// -> JSON file with all metadata
	// -> Reload to reproduce analysis

	// Option 3: Save both
	// -> Export audio for direct use
	// -> Save project for documentation
	// -> Together form complete record of analysis

	// Use cases:
	// - Audio library creation
	// - Documentation of engine characteristics
	// - QA testing of audio processing
	// - Archiving recorded data
}

/*
	==========================================================================
	EXAMPLE 9: QUICK REFERENCE - COMMON TASKS
	==========================================================================
*/

/*
	TASK: Extract engine idle from 10-minute recording
	------
	1. Open Source → select file
	2. Set Min: 50, Max: 200, Pad: 300
	3. Analyze
	4. Result: Shows number of idle segments, total duration
	5. Play to verify results
	6. Save Output → "engine_idle.wav"
	7. Save Project → "idle_analysis.json"
	Time: ~5 seconds

	TASK: Find acceleration events
	------
	1. File already loaded from previous analysis
	2. Set Min: 500, Max: 3000, Pad: 100
	3. Analyze
	4. Result: Each segment = acceleration event
	5. Save Output → "accelerations.wav"
	Time: ~3 seconds

	TASK: Compare two recordings
	------
	1. Analyze recording 1 with parameters
	2. Note results (segments, duration)
	3. Save Project → "recording1.json"
	4. Open Source → load recording 2
	5. Apply same parameters (manually or from notes)
	6. Analyze recording 2
	7. Compare results (segments, duration, frequencies)
	Time: ~10 seconds

	TASK: Create comprehensive SFX library
	------
	1. Load master recording
	2. First pass: Idle (50-200 Hz) → save "idle.wav"
	3. Second pass: Cruise (1000-2000 Hz) → save "cruise.wav"
	4. Third pass: Acceleration (2000-4000 Hz) → save "accel.wav"
	5. Fourth pass: High RPM (4000-8000 Hz) → save "high_rpm.wav"
	6. Fifth pass: Detonation (5000-10000 Hz) → save "knock.wav"
	7. Save master project for documentation
	Time: ~30 seconds total
*/

/*
	==========================================================================
	EXAMPLE 10: ADVANCED - SEGMENT METADATA INSPECTION
	==========================================================================
*/

void inspectSegmentMetadata()
{
	// After analysis, m_foundSegments contains:
	// - startSample: Sample position in source buffer
	// - endSample: End position in source buffer
	// - dominantFrequency: Peak frequency in segment
	// - magnitude: Strength at peak frequency

	// Example: Log all segments
	for (size_t i = 0; i < m_foundSegments.size(); ++i)
	{
		const auto& seg = m_foundSegments[i];
		
		double startTime = seg.startSample / (double)m_sampleRate;
		double endTime = seg.endSample / (double)m_sampleRate;
		double duration = seg.getDuration() / (double)m_sampleRate;

		// Log segment information
		// std::cout << "Segment " << i+1 << ":" << std::endl;
		// std::cout << "  Time: " << startTime << " - " << endTime << " s" << std::endl;
		// std::cout << "  Duration: " << duration << " s" << std::endl;
		// std::cout << "  Frequency: " << seg.dominantFrequency << " Hz" << std::endl;
		// std::cout << "  Magnitude: " << seg.magnitude << std::endl;
	}

	// Use cases:
	// - Validate analysis results
	// - Create segment reports
	// - Filter segments by properties
	// - Export segment metadata to CSV
}

/*
	==========================================================================
	KEYBOARD SHORTCUTS (Future Enhancement)
	==========================================================================
	
	Ctrl+O      Open source file
	Ctrl+N      New project
	Ctrl+L      Load project  
	Ctrl+S      Save project
	Space       Play/Stop playback
	Delete      Clear current project
	
	These can be implemented in a future update.
*/

/*
	==========================================================================
	PERFORMANCE TIPS
	==========================================================================

	1. Large File Processing:
	   - Analyze takes ~5-10 ms per minute of audio
	   - 1 hour file: ~5-10 seconds
	   - If too slow, reduce FFT size (less accurate)

	2. Memory Efficiency:
	   - Output buffer only stores extracted segments
	   - If many segments found, memory usage increases
	   - Typical: output ~10-20% of input size

	3. Frequency Range Tips:
	   - Narrower ranges: Faster, more specific
	   - Wider ranges: Slower, more inclusive
	   - Start wide, then narrow based on results

	4. Padding Strategy:
	   - Larger padding = larger output file
	   - Minimize padding for tight segment boundaries
	   - Use larger padding for smooth transitions

	5. Project Files:
	   - Save often for reproducibility
	   - Projects are small (1-10 KB typically)
	   - Easy to share and version control
*/
