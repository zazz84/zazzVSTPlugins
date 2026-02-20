#include "../../../zazzVSTPlugins/Shared/Filters/HilbertFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/BiquadFilters.h"

class PhaseDistortion
{
public:
	PhaseDistortion() = default;
	~PhaseDistortion() = default;

	inline void init(int sampleRate) noexcept
	{
		m_hilbert.init(sampleRate);
		m_filter.init(sampleRate);
	}
	inline void set(const float amount, const float filterGain) noexcept
    {
		m_hilbert.set();
		m_filter.setHighShelf(880.0f, 0.7f, filterGain);

		k = amount;
    }
	inline float process(float x) noexcept
    {
        // Quadrature component
		float r = 0.0f;
		float i = 0.0f;

        m_hilbert.process(x, r, i);

        // Instantaneous amplitude
		const float amplitude = sqrtf(r * r + i * i);

        // Instantaneous phase
		float phase = atan2f(i, r);

        // Phase distortion
		phase = phase + k * sinf(phase);

		phase = m_filter.processDF1(phase);

        // Reconstruct
        return amplitude * cosf(phase);
    }

private:
	HilbertFilter m_hilbert;
	BiquadFilter m_filter;
    float k = 1.0f;
};
