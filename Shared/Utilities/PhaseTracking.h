// Based on https://github.com/alexandrefrancois/Oscillators/tree/main/Sources/Oscillators

#include <cmath>

class Resonator {
public:
    // Existing members: alpha, beta, cc, ss, frequency, sampleRate, etc.

    // Compute instantaneous input phase in radians
    // Returns phase in [-pi, pi]
    float inputPhase(float sample, int sampleIndex) {
        // 1. Compute phasor at oscillator frequency
        float phasorPhase = 2.0f * M_PI * frequency * sampleIndex / sampleRate;
        float Zc = std::cos(phasorPhase);
        float Zs = std::sin(phasorPhase);

        // 2. Update smoothed resonator output
        float alphaSample = alpha * sample;
        c  = (1.0f - alpha) * c  + alphaSample * Zc;
        s  = (1.0f - alpha) * s  + alphaSample * Zs;
        cc = (1.0f - beta)  * cc + beta * c;
        ss = (1.0f - beta)  * ss + beta * s;

        // 3. Phase relative to oscillator
        float relPhase = std::atan2(ss, cc);

        // 4. Absolute input phase
        float absPhase = relPhase + phasorPhase;

        // Wrap to [-pi, pi]
        while (absPhase > M_PI)  absPhase -= 2.0f * M_PI;
        while (absPhase < -M_PI) absPhase += 2.0f * M_PI;

        return absPhase;
    }

private:
    float alpha = 0.05f;
    float beta  = 0.05f;
    float c = 0.0f;
    float s = 0.0f;
    float cc = 0.0f;
    float ss = 0.0f;
    float frequency = 1000.0f;   // oscillator freq
    float sampleRate = 48000.0f; // Hz
};

/*#include <cmath>
#include <utility>  // for std::pair

class Resonator {
public:
    // Computes input phase and instantaneous frequency
    // Returns pair {phase, frequency} in radians and Hz
    std::pair<float,float> inputPhaseAndFrequency(float sample, int sampleIndex) {
        // 1. Phasor at oscillator frequency
        float phasorPhase = 2.0f * M_PI * frequency * sampleIndex / sampleRate;
        float Zc = std::cos(phasorPhase);
        float Zs = std::sin(phasorPhase);

        // 2. Update resonator state (LPF)
        float alphaSample = alpha * sample;
        c  = (1.0f - alpha) * c  + alphaSample * Zc;
        s  = (1.0f - alpha) * s  + alphaSample * Zs;
        cc = (1.0f - beta)  * cc + beta * c;
        ss = (1.0f - beta)  * ss + beta * s;

        // 3. Relative phase of input
        float relPhase = std::atan2(ss, cc);

        // 4. Absolute phase
        float absPhase = relPhase + phasorPhase;
        while (absPhase > M_PI)  absPhase -= 2.0f * M_PI;
        while (absPhase < -M_PI) absPhase += 2.0f * M_PI;

        // 5. Frequency estimate from phase drift
        float phaseDrift = absPhase - prevPhase;
        // wrap to [-pi, pi]
        if (phaseDrift >  M_PI) phaseDrift -= 2.0f*M_PI;
        if (phaseDrift < -M_PI) phaseDrift += 2.0f*M_PI;

        float freqMeasured = phaseDrift * sampleRate / (2.0f * M_PI);

        prevPhase = absPhase;

        return {absPhase, freqMeasured};
    }

private:
    float alpha = 0.05f;
    float beta  = 0.05f;
    float c = 0.0f;
    float s = 0.0f;
    float cc = 0.0f;
    float ss = 0.0f;

    float frequency = 1000.0f;   // oscillator frequency
    float sampleRate = 48000.0f; // Hz

    float prevPhase = 0.0f;      // for frequency computation
};
*/