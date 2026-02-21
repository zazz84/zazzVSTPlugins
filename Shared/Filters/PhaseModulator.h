#include "../../../zazzVSTPlugins/Shared/Filters/HilbertFilter.h"
#include "../../../zazzVSTPlugins/Shared/Oscillators/SinOscillator.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

/*class PhaseModulator
{
public:
	PhaseModulator() = default;
	~PhaseModulator() = default;

	static constexpr float PI = 3.14159265358979f;
	static constexpr float PI2 = 2.0f * 3.14159265358979f;

	inline void init(int sampleRate) noexcept
	{
		m_hilbert.init(sampleRate);
		m_lfo.init(sampleRate);
	}
	inline void set(float modulationFrequency, float modulationDepth) noexcept
	{
		m_lfo.set(modulationFrequency);
		m_depth = PI * modulationDepth;
	}
	inline float process(float x) noexcept
	{
		// 1. Get analytic signal
		float r = 0.0f;
		float i = 0.0f;

		m_hilbert.process(x, r, i);

		float lfo = m_lfo.process();
		float mod = m_depth * lfo;

		float c = cosf(mod);
		float s = sinf(mod);

		float r2 = r * c - i * s;
		//float i2 = r * s + i * c;

		return r2; // real part
	}

private:
	HilbertFilterIIR m_hilbert;
	SinOscillator    m_lfo;

	float m_depth = 0.0f;
};*/

class PhaseModulator
{
public:
	PhaseModulator() = default;
	~PhaseModulator() = default;

	static constexpr float PI = 3.14159265358979f;
	static constexpr float PI2 = 2.0f * PI;

	inline void init(const int sampleRate) noexcept
	{
		m_hilbert.init(sampleRate);
		m_lfo.init(sampleRate);
		m_prevR = 0.0f;
	}

	inline void set(const float modulationFrequency, const float modulationDepth, const float feedback) noexcept
	{
		m_lfo.set(modulationFrequency);

		// Phase modulation depth scaled to ±π
		m_depth = PI * modulationDepth;

		m_feedback = feedback;
	}

	inline float process(const float x) noexcept
	{
		// --- 1. Real feedback injection only ---
		float in = x + m_feedback * m_prevR;

		// --- 2. Analytic signal of input+feedback ---
		float r, i;
		m_hilbert.process(in, r, i);

		// --- 3. LFO phase modulation ---
		float lfo = m_lfo.process();
		float mod = m_depth * lfo;

		float c = cosf(mod);
		float s = sinf(mod);

		// --- 4. Complex rotation ---
		float r2 = r * c - i * s;
		float i2 = r * s + i * c;

		// --- 5. Clamp to avoid oscillations ---
		r2 = Math::clamp(r2, -1.0f, 1.0f);
		
		// --- 6. Store real part for next feedback ---
		m_prevR = r2;

		return r2; // real output
	}

private:
	HilbertFilterIIR m_hilbert;
	SinOscillator    m_lfo;

	float m_depth = 0.0f;
	float m_feedback = 0.0f;

	// feedback state (real only)
	float m_prevR = 0.0f;
};