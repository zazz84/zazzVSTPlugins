# Phase Tracking and Unwrapping

## Overview
The Spectrum class now provides robust phase tracking using FFT-based I/Q interpolation with optional multi-bin phase averaging for improved stability and noise reduction.

### Phase Windowing Concept
Instead of extracting phase from only the dominant frequency bin, you can average phase from multiple nearby bins:

```
Bin:      [n-2]  [n-1]  [n]★  [n+1]  [n+2]
Mag:      0.3    0.7    1.0    0.8    0.5
Phase:    1.2    1.5    1.8    2.1    2.4
          ↓      ↓      ↓      ↓      ↓
Weighted Average (with magnitude weighting)
Result:   1.77 (smooth, stable, noise-reduced)
```

**Benefits:**
- **Noise reduction** - Averaging reduces measurement noise
- **Better for noisy signals** - Combines all nearby energy
- **Smoother transitions** - Less abrupt changes at bin boundaries
- **More physical** - Captures phase of the entire spectral peak


## Key Concepts

### Raw Phase from FFT
The FFT extracts phase directly using `atan2(Imaginary, Real)` from the I/Q (quadrature) components. This phase is:
- **Wrapped** to [-π, π] (inherent behavior of atan2)
- **Discontinuous** across frame boundaries due to wrapping
- **Bin-specific** - depends on which frequency bin contains the dominant component

### Phase Unwrapping Problem
When analyzing a frequency sweep:
1. Phase jumps by 2π when transitioning from π to -π
2. Phase appears to "jump" when tracking switches between adjacent frequency bins
3. Simple unwrapping (fixing 2π wraps) isn't enough - you need context

## Solution: Phase Windowing + Frequency-Aware Unwrapping

**Step 1: Extract phase with windowing** (optional multi-bin averaging)

```cpp
// Use phaseWindowSize=2 for stable, smooth phase
Spectrum::calculateDominantFrequenciesWithIQInterpolation(
    buffer,
    sampleRate,
    dominantFrequencies,
    timeBinCenterSamples,
    &phaseTrajectory,
    512, 12,
    2);  // ±2 bins = 5-bin average
```

**Step 2: Unwrap with frequency awareness**

```cpp
std::vector<float> unwrappedPhase;
Spectrum::unwrapPhaseTrajectoryWithFrequency(
    phaseTrajectory,          // Raw phase from FFT
    dominantFrequencies,      // Frequencies at each frame
    frameDurationSeconds,     // How long each frame lasts
    unwrappedPhase);          // Output continuous phase
```

**How it works:**
1. Calculates expected phase advance based on frequency: `Δphase = 2π × f × Δt`
2. Finds the unwrapped version of measured phase **closest to expected**
3. Maintains continuity even across bin changes

### Complete Example

```cpp
// Analyze audio
std::vector<float> dominantFrequencies;
std::vector<int> timeBinCenterSamples;
std::vector<float> phaseTrajectory;

// Extract frequency and phase (5-bin averaged)
Spectrum::calculateDominantFrequenciesWithIQInterpolation(
    buffer,
    48000,  // sampleRate
    dominantFrequencies,
    timeBinCenterSamples,
    &phaseTrajectory,
    512,    // binsPerSecond
    12,     // fftOrder
    2);     // phaseWindowSize=2 for stability

// Unwrap phase using frequency information
std::vector<float> unwrappedPhase;
float frameDuration = (float)dominantFrequencies.size() / 48000;

Spectrum::unwrapPhaseTrajectoryWithFrequency(
    phaseTrajectory,
    dominantFrequencies,
    frameDuration,
    unwrappedPhase);

// Now unwrappedPhase is continuous and smooth!
```

## What You'll See

**Before unwrapping:**
```
Raw phase: 0.5, 1.0, -2.8, 2.5, 3.0, -1.2, 0.8...  (full of jumps)
```

**After unwrapping with frequency context:**
```
Unwrapped: 0.5, 1.0, 3.44, 3.75, 4.25, 5.08, 5.95...  (smooth progression)
```

The unwrapped phase increases monotonically or decreases smoothly, as expected for a real signal.

## Phase Extraction Methods

### Single-Bin Phase
```cpp
calculateDominantFrequencies(..., phaseWindowSize = 0)
calculateDominantFrequenciesWithIQInterpolation(..., phaseWindowSize = 0)
```
Extracts phase from only the dominant bin with optional sub-bin interpolation.

### Multi-Bin Averaged Phase (Recommended)
```cpp
calculateDominantFrequencies(..., phaseWindowSize = 2)  // ±2 bins = 5-bin average
calculateDominantFrequenciesWithIQInterpolation(..., phaseWindowSize = 2)
```
Averages phase from a window of bins around the dominant:
- **More stable** - noise reduction through averaging
- **Better SNR** - combines energy from nearby bins
- **Smoother** - reduces abrupt changes when bin changes
- **Magnitude-weighted** - stronger bins contribute more

## Method Reference

### Phase Averaging
`averagePhaseFromWindow(centerPhase, phasesAroundCenter, magnitudesAroundCenter, numBinsAround)`
- Properly unwraps phases relative to center before averaging
- Uses magnitude-squared weighting for proper emphasis
- Returns wrapped result in [-π, π]

### Unwrapping

#### `unwrapPhaseTrajectory(inPhase, outUnwrapped, threshold)`
Simple unwrapping - just fixes 2π wraps. Good for:
- Single-bin phase
- Static sine tones
- Quick checks

#### `unwrapPhaseTrajectoryWithFrequency(inPhase, frequencies, frameDuration, outUnwrapped)`
Smart unwrapping - considers frequency evolution. Good for:
- Multi-bin averaged phase
- Frequency sweeps
- Bin changes
- Accurate phase tracking
- Phase vocoder applications

## Typical Issues and Fixes

| Issue | Cause | Fix |
|-------|-------|-----|
| Phase 10x slower than expected | Using raw wrapped phase | Use `unwrapPhaseTrajectoryWithFrequency()` |
| Jumps at frequency transitions | 2π wraps at bin changes | Use multi-bin averaging with phaseWindowSize=2 |
| Noisy phase variations | Single-bin extraction | Use phaseWindowSize=2 or higher for averaging |
| Wrong phase rate with sweep | Principal phase was subtracted incorrectly | Don't use principal phase correction |

## Implementation Details

- **I/Q Interpolation** - Interpolates real/imaginary components before phase calculation for sub-bin accuracy
- **Raw Phase Output** - Phase is output as measured from FFT (wrapped), unwrapping is done separately
- **Frequency Integration** - Unwrapping uses measured frequency to predict expected phase advance


