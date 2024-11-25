#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"
#include "../../../zazzVSTPlugins/Shared/Utilities/Math3D.h"
#include "../../../zazzVSTPlugins/Shared/Reverbs/EarlyReflections.h"
#include "../../../zazzVSTPlugins/Shared/Filters/OnePoleFilters.h"

#include <vector>
#include <cmath>

#define M_PI  3.14159268f
#define SPEED_OF_SOUND 343.0f

//==============================================================================
class FDNReverbAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    FDNReverbAudioProcessor();
    ~FDNReverbAudioProcessor() override;

	static const std::string paramsNames[];
	static const std::string paramsUnitNames[];
	static const float LATE_REFLECTION_DELAY_TIME_NORMALIZED[];
	static const float MAXIMUM_DELAY_TIME;
	static const float AVERAGE_DELAY_TIME;
	static const float MAXIMUM_ROOM_DIMENSION;
	static const int DELAY_LINES_COUNT = 16;
	static const int MAXIMUM_REFLECTIONS_COUNT = 32;
	static constexpr float MAXIMUM_PREDELAY_MS = 50;

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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================
	float remap(float value, float inMin, float inMax, float outMin, float outMax)
	{
		if (value <= inMin)
		{
			return outMin;
		}
		else if (value >= inMax)
		{
			return outMax;
		}
		else
		{
			return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
		}
	}

	//==============================================================================
	// Input size must be a power of 2.
	void FWHT(float (&data)[DELAY_LINES_COUNT])
	{
		int h = 1;

		// Iterative Fast Walsh-Hadamard Transform
		while (h < DELAY_LINES_COUNT)
		{
			for (int i = 0; i < DELAY_LINES_COUNT; i += h * 2)
			{
				for (int j = i; j < i + h; ++j)
				{
					float x = data[j];
					float y = data[j + h];
					data[j] = x + y;
					data[j + h] = x - y;
				}
			}

			// Normalize by sqrt(2) at each step
			for (int i = 0; i < DELAY_LINES_COUNT; ++i)
			{
				data[i] *= 0.707f;
			}

			h *= 2;
		}
	}
	
	struct Params
	{
		// Default constructor
		Params()
			: L(0.0f), W(0.0f), H(0.0f),
			reflections(0.0f), ERDamping(0.0f), ERWidth(0.0f),
			LRPredelay(0.0f), time(0.0f), size(0.0f),
			LRDamping(0.0f), color(0.0f), LRWidth(0.0f),
			ERvolume(0.0f), LRvolume(0.0f), mix(0.0f), volume(0.0f)
		{}

		// Parameterized constructor
		Params(float l, float w, float h,
			float reflections, float erDamping, float erWidth,
			float lrPredelay, float time, float size,
			float lrDamping, float color, float lrWidth,
			float erVolume, float lrVolume, float mix, float volume)
			: L(l), W(w), H(h),
			reflections(reflections), ERDamping(0.01f * erDamping), ERWidth(0.01f * erWidth),
			LRPredelay(lrPredelay), time(time), size(size),
			LRDamping(0.01f * lrDamping), color(color), LRWidth(0.01f * lrWidth),
			ERvolume(erVolume), LRvolume(lrVolume), mix(0.01f * mix), volume(volume) {}

		float L, W, H;         // Dimensions
		float reflections;
		float ERDamping;
		float ERWidth;
		float LRPredelay;
		float time;            // Reverb time parameter [s]
		float size;                 
		float LRDamping;       // Damping parameter
		float color;           // Color parameter
		float LRWidth;          // Width parameter
		float ERvolume;
		float LRvolume;
		float mix;             // Mix parameter
		float volume;          // Volume parameter
	};

	FibonacciSphereEarlyReflections m_earlyReflections[2];

	CircularBuffer m_bufferLR[2][DELAY_LINES_COUNT];
	CircularBuffer m_predelay[2];
	OnePoleLowPassFilter m_filterLR[2][DELAY_LINES_COUNT];
	BiquadFilter m_highPass[2];
	BiquadFilter m_lowShelf[2];
	BiquadFilter m_highShelf[2];

	float m_gainsLR[2][DELAY_LINES_COUNT];
	float m_tmp[DELAY_LINES_COUNT];
	
	Params m_Params;

	// Early reflections
	std::atomic<float>* LParameter = nullptr;
	std::atomic<float>* WParameter = nullptr;
	std::atomic<float>* HParameter = nullptr;
	std::atomic<float>* ReflectionsParameter = nullptr;
	std::atomic<float>* ERDampingParameter = nullptr;
	std::atomic<float>* ERWidthParameter = nullptr;
		
	// Late reflections
	std::atomic<float>* LRPredelayParameter = nullptr;
	std::atomic<float>* timeParameter = nullptr;
	std::atomic<float>* sizeParameter = nullptr;
	std::atomic<float>* LRDampingParameter = nullptr;
	std::atomic<float>* colorParameter = nullptr;
	std::atomic<float>* LRWidthParameter = nullptr;
	
	// Mix
	std::atomic<float>* ERVolumeParameter = nullptr;
	std::atomic<float>* LRVolumeParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	float m_feedback;

	//==============================================================================
	void OnParamsChanged(Params& params)
	{
		const float sampleRate = (float)getSampleRate();
		const float maximumLRSize = MAXIMUM_DELAY_TIME * sampleRate;
		const float LRWidthFactor = 1.0f - (0.05f * params.LRWidth);
		const float LRTimeFactor = 0.4f + 0.6f * params.size;

		const float sampleRateHalf = 0.5f * sampleRate;
		float maximumFilterFrequency = (sampleRateHalf < 15000) ? sampleRateHalf : 15000.0f;
		maximumFilterFrequency = remap(params.LRDamping, 0.0f, 1.0f, maximumFilterFrequency, 1000.0f);

		// Calculate listener position for left and right channels
		Point3D listenerPosition[2];

		const float offsetY = 0.5f * params.W * params.ERWidth;

		listenerPosition[0].x = 0.5f;
		listenerPosition[0].y = 0.5f - offsetY;
		listenerPosition[0].z = 0.5f;

		listenerPosition[1].x = 0.5f;
		listenerPosition[1].y = 0.5f + offsetY;
		listenerPosition[1].z = 0.5f;

		for (int channel = 0; channel < 2; channel++)
		{
			Point3D listener;

			m_earlyReflections[channel].set(params.L, params.W, params.H, params.ERDamping, listenerPosition[channel], (int)params.reflections);

			m_predelay[channel].setSize((int)((float)sampleRate * params.LRPredelay * 0.001f));

			for (int delayLine = 0; delayLine < DELAY_LINES_COUNT; delayLine++)
			{
				// Late reflections
				// Gain
				constexpr auto a = 10.0f;
				const float timeLR = MAXIMUM_DELAY_TIME * LATE_REFLECTION_DELAY_TIME_NORMALIZED[delayLine];
				const float distanceLR = timeLR * SPEED_OF_SOUND;
				m_gainsLR[channel][delayLine] = a / (distanceLR + a);
				
				//Width
				if (channel == 0)
				{
					m_bufferLR[channel][delayLine].setSize((int)(LRTimeFactor * maximumLRSize * LATE_REFLECTION_DELAY_TIME_NORMALIZED[delayLine]));
				}
				else if (channel == 1)
				{
					m_bufferLR[channel][delayLine].setSize((int)(LRWidthFactor * LRTimeFactor * maximumLRSize * LATE_REFLECTION_DELAY_TIME_NORMALIZED[delayLine]));
				}
				
				// Damping filter
				const auto frequency = remap(distanceLR, 2.0f, 50.0f, maximumFilterFrequency, 500.0f);
				m_filterLR[channel][delayLine].set(frequency);
			}
		}

		const float averageDelayTime = LRTimeFactor * AVERAGE_DELAY_TIME;
		m_feedback = std::expf((-6.9078f * averageDelayTime * MAXIMUM_DELAY_TIME) / params.time);
	}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNReverbAudioProcessor)
};
