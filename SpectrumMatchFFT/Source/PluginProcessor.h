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

#include <array>
#include <vector>
#include <map>

#include <JuceHeader.h>

#include "SpectrumMatchFFTDetect.h"
#include "SpectrumMatchFFTApply.h"

//==============================================================================


//==============================================================================
class SpectrumMatchFFTAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    SpectrumMatchFFTAudioProcessor();
    ~SpectrumMatchFFTAudioProcessor() override;

	enum Parameters
    {
        LowPassFilter,
		HighPassFilter,
		FrequencyShift,
		Resolution,
		Ammount,
		Volume,
        COUNT
    };

    static const std::string paramsNames[];
    static const std::string labelNames[];
	static const std::string paramsUnitNames[];
    static const int N_CHANNELS = 2;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
	struct NoteBin
	{
		int index;   // grouped semitone index
		float energy; // accumulated energy
	};

	// Convert FFT magnitude -> grouped note bins
	std::vector<NoteBin> fftToNoteBins(
		const std::vector<float>& fftMagnitude, // N/2 + 1
		float sampleRate,
		int fftSize,
		int semitoneResolution = 1,              // 1=semitone, 2=whole tone, 12=octave
		float referenceFreq = 440.0f             // A4
	)
	{
		const int numBins = fftMagnitude.size();
		const float binHz = sampleRate / fftSize;

		std::map<int, float> noteEnergy;

		for (int k = 1; k < numBins; ++k) // skip DC
		{
			float freq = k * binHz;
			if (freq <= 0.0f)
				continue;

			// Continuous semitone index
			float semitoneFloat = 12.0f * std::log2(freq / referenceFreq);

			// Quantized semitone
			int semitone = static_cast<int>(std::floor(semitoneFloat + 0.5f));

			// Group by resolution
			int groupIndex = semitone / semitoneResolution;

			// Accumulate energy (magnitude^2)
			float energy = fftMagnitude[k] * fftMagnitude[k];
			noteEnergy[groupIndex] += energy;
		}

		std::vector<NoteBin> result;
		result.reserve(noteEnergy.size());

		for (const auto& kv : noteEnergy)
		{
			result.push_back({ kv.first, kv.second });
		}

		return result;
	}

	// Convert grouped semitone index -> center frequency
	inline float noteBinToFreq(int groupIndex, int resolution, float referenceFreq = 440.0f)
	{
		int semitone = groupIndex * resolution;
		return referenceFreq * std::pow(2.0f, semitone / 12.0f);
	}

	// Inverse: create FFT magnitude array from note bins
	std::vector<float> noteBinsToFFT(
		const std::vector<NoteBin>& noteBins,
		float sampleRate,
		int fftSize,
		int semitoneResolution = 1,
		float referenceFreq = 440.0f
	)
	{
		int numBins = fftSize / 2 + 1;
		std::vector<float> fftMagnitude(numBins, 0.0f);

		if (noteBins.empty())
			return fftMagnitude;

		// Sort noteBins by index
		std::vector<NoteBin> sortedBins = noteBins;
		std::sort(sortedBins.begin(), sortedBins.end(),
			[](const NoteBin& a, const NoteBin& b) { return a.index < b.index; });

		// Precompute center frequencies and magnitudes
		std::vector<float> binFreqs(sortedBins.size());
		std::vector<float> binMagnitudes(sortedBins.size());
		for (size_t i = 0; i < sortedBins.size(); ++i)
		{
			binFreqs[i] = noteBinToFreq(sortedBins[i].index, semitoneResolution, referenceFreq);
			binMagnitudes[i] = std::sqrt(sortedBins[i].energy); // energy -> magnitude
		}

		// Linear interpolation in log-frequency
		for (int k = 1; k < numBins; ++k) // skip DC
		{
			float f = k * sampleRate / fftSize;
			if (f <= 0.0f)
				continue;

			float logF = std::log(f);

			// Find surrounding bins
			size_t i = 0;
			while (i + 1 < binFreqs.size() && std::log(binFreqs[i + 1]) < logF)
				++i;

			if (i + 1 >= binFreqs.size())
			{
				fftMagnitude[k] = binMagnitudes.back();
			}
			else
			{
				float f1 = std::log(binFreqs[i]);
				float f2 = std::log(binFreqs[i + 1]);
				float m1 = binMagnitudes[i];
				float m2 = binMagnitudes[i + 1];

				float t = (logF - f1) / (f2 - f1);
				fftMagnitude[k] = m1 + t * (m2 - m1);
			}
		}

		return fftMagnitude;
	}

	// Shift FFT magnitude spectrum by shiftHz (can be positive or negative)
	std::vector<float> shiftFftMagnitudeHz(
		const std::vector<float>& inMagnitude, // N/2 + 1
		float sampleRate,
		int fftSize,
		float shiftHz
	)
	{
		const int numBins = fftSize / 2 + 1;
		const float binHz = sampleRate / fftSize;

		std::vector<float> outMagnitude(numBins, 0.0f);

		if (shiftHz == 0.0f)
			return inMagnitude;

		for (int k = 0; k < numBins; ++k)
		{
			// Target frequency of output bin
			float targetFreq = k * binHz;

			// Source frequency before shift
			float sourceFreq = targetFreq - shiftHz;

			// Out of range → zero
			if (sourceFreq < 0.0f)
			{
				outMagnitude[k] = inMagnitude[1];
			}		
				
			if (sourceFreq > sampleRate * 0.5f)
			{
				outMagnitude[k] = inMagnitude.back();
			}

			// Convert source frequency to fractional bin index
			float srcIndex = sourceFreq / binHz;

			int i0 = static_cast<int>(std::floor(srcIndex));
			int i1 = i0 + 1;

			if (i0 < 0 || i1 >= numBins)
				continue;

			float frac = srcIndex - i0;

			// Linear interpolation
			float v0 = inMagnitude[i0];
			float v1 = inMagnitude[i1];
			outMagnitude[k] = v0 + frac * (v1 - v0);
		}

		return outMagnitude;
	}

	std::vector<float> shiftFftMagnitudeMul(
		const std::vector<float>& inMagnitude, // N/2 + 1
		float sampleRate,
		int fftSize,
		float multiplier
	)
	{
		const int numBins = fftSize / 2 + 1;
		const float binHz = sampleRate / fftSize;

		std::vector<float> outMagnitude(numBins, 0.0f);

		if (multiplier <= 0.0f || multiplier == 1.0f)
			return inMagnitude;

		for (int k = 0; k < numBins; ++k)
		{
			// Target frequency of output bin
			float targetFreq = k * binHz;

			// Source frequency (scaled)
			float sourceFreq = targetFreq / multiplier;

			// Clamp low
			if (sourceFreq < 0.0f)
			{
				outMagnitude[k] = inMagnitude[1];
				continue;
			}

			// Clamp high
			if (sourceFreq > sampleRate * 0.5f)
			{
				outMagnitude[k] = inMagnitude.back();
				continue;
			}

			// Convert source frequency to fractional bin index
			float srcIndex = sourceFreq / binHz;

			int i0 = static_cast<int>(std::floor(srcIndex));
			int i1 = i0 + 1;

			if (i0 < 0 || i1 >= numBins)
				continue;

			float frac = srcIndex - i0;

			// Linear interpolation
			float v0 = inMagnitude[i0];
			float v1 = inMagnitude[i1];
			outMagnitude[k] = v0 + frac * (v1 - v0);
		}

		return outMagnitude;
	}

	void apply24dBButterworth(
		std::vector<float>& spectrumMultipliers,
		double sampleRate,
		int fftSize,
		float cutoffHz,
		bool highPass)
	{
		const int numBins = static_cast<int>(spectrumMultipliers.size());

		for (int bin = 0; bin < numBins; ++bin)
		{
			const double freq = (double)bin * sampleRate / fftSize;

			if (freq <= 0.0)
				continue;

			const double ratio = highPass ? (cutoffHz / freq)
				: (freq / cutoffHz);

			// 4th order Butterworth → 24 dB/oct
			const double magnitude = 1.0 / std::sqrt(1.0 + std::pow(ratio, 8.0));

			const double adjustment = (1.0f - spectrumMultipliers[bin]) * (1.0f - magnitude);

			spectrumMultipliers[bin] += static_cast<float>(adjustment);
		}
	}
	
	//==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

	//==============================================================================
	int getNumBins()
	{
		return m_fftSize / 2 + 1;
	}
	int getFFTSize()
	{
		return m_fftSize;
	}
	const std::vector<float>& const getSourceSpectrum()
	{
		return m_sourceSpectrumRuntime;
	}
	const std::vector<float>& const getTargetSpectrum()
	{
		return m_targetSpectrumRuntime;
	}
	const std::vector<float>& const getFilterSpectrum()
	{
		return m_filterSpectrumRuntime;
	}

private:	
	//==============================================================================
	/** fftSize must be a power of two (e.g. 512, 1024, 2048, 4096) */
	static constexpr int m_fftSize = 4096;
	
	void storeSourceSpectrum()
	{
		juce::MemoryBlock block(m_spectrumDetect[0].getSpectrum(), getNumBins() * sizeof(float));

		apvts.state.setProperty("SourceSpectrumData", block, nullptr);
	}
	void loadSourceSpectrum()
	{
		if (!apvts.state.hasProperty("SourceSpectrumData"))
			return;

		auto* block = apvts.state.getProperty("SourceSpectrumData").getBinaryData();

		if (block == nullptr)
			return;

		const size_t numFloats = block->getSize() / sizeof(float);
		m_sourceSpectrum.resize(numFloats);

		std::memcpy(m_sourceSpectrum.data(), block->getData(), block->getSize());
	}
	void storeTargetSpectrum()
	{
		juce::MemoryBlock block(m_spectrumDetectTarget[0].getSpectrum(), getNumBins() * sizeof(float));

		apvts.state.setProperty("TargetSpectrumData", block, nullptr);
	}
	void loadTargetSpectrum()
	{
		if (!apvts.state.hasProperty("TargetSpectrumData"))
			return;

		auto* block = apvts.state.getProperty("TargetSpectrumData").getBinaryData();

		if (block == nullptr)
			return;

		const size_t numFloats = block->getSize() / sizeof(float);
		m_targetSpectrum.resize(numFloats);

		std::memcpy(m_targetSpectrum.data(), block->getData(), block->getSize());
	}
	void updateSourceSpectrum(const float frequencyShift, const float resolution)
	{
		std::vector<float> shiftedSourceSpectrum = shiftFftMagnitudeMul(m_sourceSpectrum, getSampleRate(), getFFTSize(), frequencyShift);
		std::vector<SpectrumMatchFFTAudioProcessor::NoteBin> noteBinsSource = fftToNoteBins(shiftedSourceSpectrum, getSampleRate(), getFFTSize(), resolution);
		m_sourceSpectrumRuntime.clear();
		m_sourceSpectrumRuntime = noteBinsToFFT(noteBinsSource, getSampleRate(), getFFTSize(), resolution);
	}
	void updateTargetSpectrum(const float resolution)
	{
		std::vector<SpectrumMatchFFTAudioProcessor::NoteBin> noteBinsTarget = fftToNoteBins(m_targetSpectrum, getSampleRate(), getFFTSize(), resolution);
		m_targetSpectrumRuntime.clear();
		m_targetSpectrumRuntime = noteBinsToFFT(noteBinsTarget, getSampleRate(), getFFTSize(), resolution);
	}
	void updateFilterSpectrum(const float ammount, const float highpassFilter, const float lowPassFilter)
	{
		// Get runtime filter spectrum
		jassert(m_sourceSpectrumRuntime.size() == m_targetSpectrumRuntime.size());

		m_filterSpectrumRuntime.clear();
		m_filterSpectrumRuntime.resize(m_sourceSpectrumRuntime.size());

		const float ammountMultiplier = 0.01f * ammount;

		for (int i = 0; i < m_sourceSpectrumRuntime.size(); i++)
		{
			float dbA = juce::Decibels::gainToDecibels(m_sourceSpectrumRuntime[i]);
			float dbB = juce::Decibels::gainToDecibels(m_targetSpectrumRuntime[i]);
			m_filterSpectrumRuntime[i] = juce::Decibels::decibelsToGain(ammountMultiplier * (dbA - dbB));
		}

		apply24dBButterworth(m_filterSpectrumRuntime, getSampleRate(), getFFTSize(), highpassFilter, true);
		apply24dBButterworth(m_filterSpectrumRuntime, getSampleRate(), getFFTSize(), lowPassFilter, false);
	}

	std::array<SpectrumMatchFFTDetect, N_CHANNELS> m_spectrumDetect{
		SpectrumMatchFFTDetect(m_fftSize),
		SpectrumMatchFFTDetect(m_fftSize)
	};

	std::array<SpectrumMatchFFTDetect, N_CHANNELS> m_spectrumDetectTarget{
		SpectrumMatchFFTDetect(m_fftSize),
		SpectrumMatchFFTDetect(m_fftSize)
	};

	std::array<SpectrumMatchFFTApply, N_CHANNELS> m_spectrumApply{
		SpectrumMatchFFTApply(m_fftSize),
		SpectrumMatchFFTApply(m_fftSize)
	};

	juce::AudioParameterBool* m_sourceSpectrumButton;
	juce::AudioParameterBool* m_targetSpectrumButton;
	std::array<std::atomic<float>*, Parameters::COUNT> m_parameters;
	std::array<float, Parameters::COUNT> m_parametersValues;

	std::vector<float> m_sourceSpectrum;
	std::vector<float> m_targetSpectrum;

	std::vector<float> m_sourceSpectrumRuntime;
	std::vector<float> m_targetSpectrumRuntime;
	std::vector<float> m_filterSpectrumRuntime;

	bool m_learnSourceLast = false;
	bool m_learnTargetLast = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumMatchFFTAudioProcessor)
};
