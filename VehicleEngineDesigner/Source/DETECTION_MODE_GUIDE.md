# Detection Mode UI Visibility Guide

This document describes which UI controls are visible for each detection mode.

## Detection Mode Visibility Matrix

```
┌──────────────────────┬───────┬────────┬─────┬────────────┐
│ Slider/Control       │  TDom │ TDom+F │ FFT │ FFT+Filter │
│ (ID = 1, 2, 3, 4)    │   1   │   2    │  3  │     4      │
├──────────────────────┼───────┼────────┼─────┼────────────┤
│ threshold            │   ✓   │   ✓    │  -  │     -      │
│ minimumLength        │   ✓   │   ✓    │  ✓  │     -      │
│ minimumLengthMult    │   -   │   -    │  -  │     ✓      │
│ lowPassFrequency     │   -   │   ✓    │  -  │     -      │
│ exportMaxOffset      │   ✓   │   ✓    │  ✓  │     -      │
│ fftPhaseThreshold    │   -   │   -    │  ✓  │     -      │
└──────────────────────┴───────┴────────┴─────┴────────────┘
```

## Mode Details

### Mode 1: Time Domain
**ID: 1**
- **Visible:** threshold, minimumLength, exportMaxOffset
- **Hidden:** minimumLengthMult, lowPassFrequency, fftPhaseThreshold
- **Use Case:** Detect regions based on amplitude thresholds in the time domain

### Mode 2: Time Domain + Filter
**ID: 2**
- **Visible:** threshold, minimumLength, lowPassFrequency, exportMaxOffset
- **Hidden:** minimumLengthMult, fftPhaseThreshold
- **Use Case:** Detect regions in filtered time domain data; allows low-pass filtering before detection

### Mode 3: FFT
**ID: 3**
- **Visible:** minimumLength, fftPhaseThreshold, exportMaxOffset
- **Hidden:** threshold, minimumLengthMult, lowPassFrequency
- **Use Case:** Detect regions based on dominant frequency analysis; uses phase threshold

### Mode 4: FFT + Filter
**ID: 4**
- **Visible:** minimumLengthMult
- **Hidden:** threshold, minimumLength, lowPassFrequency, exportMaxOffset, fftPhaseThreshold
- **Use Case:** Detect regions with per-sample dominant frequency filtering; uses length multiplier instead of fixed length
- **Note:** The offset slider is hidden because region lengths are dynamic

## Implementation

The `updateDetectionModeUI()` method in `MainComponent.h` handles all visibility changes. It's called from the detection type combo box onChange handler and clearly shows which controls are visible for each mode through an easy-to-read matrix.

## Related Code

- **MainComponent.h**: `updateDetectionModeUI()` method
- **MainComponent.cpp**: Detection type combo box onChange handler
- **detectRegionsButtonClicked()**: Uses the appropriate slider based on mode
