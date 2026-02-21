/*
 * Copyright (C) 2026 Filip Cenzak (filip.c@centrum.cz)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// Based on https://github.com/Signalsmith-Audio/hilbert-iir

#pragma once
#include <array>
#include <cmath>
#include <algorithm>

class HilbertFilterIIR
{
public:
	HilbertFilterIIR() = default;
	~HilbertFilterIIR() = default;

	static constexpr int order = 12;

    inline void init(int sampleRate) noexcept
    {
        m_sampleRate = sampleRate;
        computeCoefficients();
        reset();
    }
	inline void set(float passbandGain = 1.0f) noexcept
    {
        m_passbandGain = passbandGain;
        computeCoefficients();
    }
	inline void reset() noexcept
    {
        m_stateR.fill(0.0f);
        m_stateI.fill(0.0f);
    }

    // Process single real sample
    // Outputs analytic signal: outR + j*outI
    inline void process(float in, float& outR, float& outI) noexcept
    {
        for (int i = 0; i < order; ++i)
        {
            float prevR = m_stateR[i];
            float prevI = m_stateI[i];

            float newR = prevR * m_polesR[i]
                       - prevI * m_polesI[i]
                       + in * m_coeffsR[i];

            float newI = prevR * m_polesI[i]
                       + prevI * m_polesR[i]
                       + in * m_coeffsI[i];

            m_stateR[i] = newR;
            m_stateI[i] = newI;
        }

        outR = in * m_direct;
        outI = 0.0f;

        for (int i = 0; i < order; ++i)
        {
            outR += m_stateR[i];
            outI += m_stateI[i];
        }
    }

private:
    void computeCoefficients()
    {
        // Base coefficients (real/imag split)
        constexpr float baseCoeffR[order] = {
            -0.000224352093802f,  0.0107500557815f,  -0.0456795873917f,
             0.11282500582f,     -0.208067578452f,   0.28717837501f,
            -0.254675294431f,     0.0481081835026f,  0.227861357867f,
            -0.365411839137f,     0.280729061131f,  -0.0935061787728f
        };

        constexpr float baseCoeffI[order] = {
             0.00543499018201f,  -0.0173890685681f,   0.0229166931429f,
             0.00278413661237f, -0.104628958675f,     0.33619239719f,
            -0.683033899655f,    0.954061589374f,    -0.891273574569f,
             0.525088317271f,   -0.155131206606f,     0.00512245855404f
        };

        constexpr float basePoleR[order] = {
            -0.00495335976478f, -0.017859491302f, -0.0413714373155f,
            -0.0882148408885f,  -0.17922965812f,  -0.338261800753f,
            -0.557688699732f,   -0.735157736148f, -0.719057381172f,
            -0.517871025209f,   -0.280197469471f, -0.0852751354531f
        };

        constexpr float basePoleI[order] = {
             0.0092579876872f,  0.0273493725543f,  0.0744756910287f,
             0.178349677457f,   0.39601340223f,    0.829229533354f,
             1.61298538328f,    2.79987398682f,    4.16396166128f,
             5.29724826804f,    5.99598602388f,    6.3048492377f
        };

        constexpr float baseDirect = 0.000262057212648f;

        const float freqFactor = std::min(0.46f, 20000.0f / static_cast<float>(m_sampleRate));

        m_direct = baseDirect * 2.0f * m_passbandGain * freqFactor;

        for (int i = 0; i < order; ++i)
        {
			const float coeffR = baseCoeffR[i] * freqFactor * m_passbandGain;
			const float coeffI = baseCoeffI[i] * freqFactor * m_passbandGain;

            m_coeffsR[i] = coeffR;
            m_coeffsI[i] = coeffI;

            const float pr = basePoleR[i] * freqFactor;
            const float pi = basePoleI[i] * freqFactor;

			const float expR = std::exp(pr);
			const float cosI = std::cos(pi);
			const float sinI = std::sin(pi);

            m_polesR[i] = expR * cosI;
            m_polesI[i] = expR * sinI;
        }
    }

	std::array<float, order> m_stateR{};
	std::array<float, order> m_stateI{};
	std::array<float, order> m_coeffsR{};
	std::array<float, order> m_coeffsI{};
	std::array<float, order> m_polesR{};
	std::array<float, order> m_polesI{};

    float m_direct = 0.0f;
    float m_passbandGain = 2.0f;
    int   m_sampleRate = 48000;
};

//==============================================================================
class HilbertFilterFIR
{
public:
	HilbertFilterFIR(int taps)
    {
        if (taps % 2 == 0)
            taps++;  // Must be odd

        N = taps;
        h.resize(N);
        buffer.resize(N, 0.0);

        designHilbert();
    }

    double process(double input)
    {
        // Shift buffer
        for (int i = N - 1; i > 0; --i)
            buffer[i] = buffer[i - 1];

        buffer[0] = input;

        // FIR convolution
        double y = 0.0;
        for (int i = 0; i < N; ++i)
            y += h[i] * buffer[i];

        return y;  // Quadrature output
    }

private:
    int N;
    std::vector<double> h;
    std::vector<double> buffer;

	static constexpr float PI = 3.14159265358979f;

    void designHilbert()
    {
        int M = (N - 1) / 2;

        for (int n = 0; n < N; ++n)
        {
            int k = n - M;

            if (k == 0)
            {
                h[n] = 0.0;
            }
            else if (k % 2 != 0)
            {
                h[n] = 2.0 / (PI * k);
            }
            else
            {
                h[n] = 0.0;
            }

            // Apply Hamming window
            h[n] *= (0.54 - 0.46 * cos(2.0 * PI * n / (N - 1)));
        }
    }
};