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

#pragma once

#include <algorithm>
#include <cmath>

#include "../../../zazzVSTPlugins/Shared/Filters/HighOrderBiquadFilter.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math.h"

class AdaptiveReleaseTime
{
public:
    AdaptiveReleaseTime() = default;
    ~AdaptiveReleaseTime() = default;

    static constexpr float EPSILON = 1e-6f;

    inline void init(int sampleRate) noexcept
    {
        m_lowPassFilter.init(sampleRate);
        m_lowPassFilter.set(12000.0f);

        m_slopeSmoother.init(sampleRate);
        m_slopeSmoother.set(3.0f);

        m_sampleRate = (float)sampleRate;
    }
    void set(float releaseTimeMinMS, float releaseTimeMaxMS, float shape) noexcept
    {
        m_releaseTimeMinMS = releaseTimeMinMS;
        m_releaseTimeMaxMS = releaseTimeMaxMS;
        
        // shape = 1.0 → linear(neutral)
        // shape < 1.0 → faster reaction(more punchy)
        // shape > 1.0 → smoother / slower
        m_shape = shape;
    }
    void setThreshold(const float threshold)
    {
        m_threshold = threshold;
    }
    void release() noexcept
    {
        m_lowPassFilter.release();
        m_slopeSmoother.release();
        
        m_x1 = 0.0f;
        m_releaseTimeLast = 0.0f;
        m_sampleRate = 0.0f;

        m_releaseTimeMinMS = 0.1f;
        m_releaseTimeMaxMS = 100.0f;
        m_shape = 1.0f;
    }
    float process(float in) noexcept
    {
        // Filter
        const float inFiltered = m_lowPassFilter.process(in);
        
        // Calculate slope
        float slope = inFiltered - m_x1;
        m_x1 = inFiltered;

        // Process only raising slopes
        if (slope > EPSILON)
        {
            /*slope = m_slopeSmoother.process(slope);
            slope = std::fminf(slope, 1.0f);

            // Apply shape curve
            slope = std::pow(slope, m_shape);

            // Remap
            m_releaseTimeLast = Math::remap(slope, 0.0f, 1.0f, m_releaseTimeMaxMS, m_releaseTimeMinMS);*/


            slope = m_slopeSmoother.process(slope);
            const float frequency = Math::slopeToFrequency(slope, m_threshold, m_sampleRate);

            // Trying to make release time equal half of the period, when using sin signal input
            constexpr float MAGIC_NUMBER = 1.5f;

            m_releaseTimeLast = (MAGIC_NUMBER * 1000.0f) / (frequency + EPSILON);
            m_releaseTimeLast = Math::clamp(m_releaseTimeLast, m_releaseTimeMinMS, m_releaseTimeMaxMS);
        }

         return m_releaseTimeLast;
    }

private:
    ForthOrderLowPassFilter m_lowPassFilter;
    OnePoleLowPassFilter m_slopeSmoother;

    float m_x1 = 0.0f;
    float m_releaseTimeLast = 0.0f;
    float m_sampleRate = 0.0f;
    float m_threshold = 1.0f;

    float m_releaseTimeMinMS = 0.1f;
    float m_releaseTimeMaxMS = 100.0f;
    float m_shape = 1.0f;
};