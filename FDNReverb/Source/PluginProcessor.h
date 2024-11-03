#pragma once

#include <JuceHeader.h>
#include "../../../zazzVSTPlugins/Shared/Utilities/CircularBuffers.h"

#include <vector>
#include <cmath>

#define M_PI  3.14159268f

//==============================================================================
class OnePoleFilter
{
public:
	void init(const float sampleRate)
	{
		m_sampleRate = sampleRate;
	};
	void set(const float frequency)
	{
		m_a0 = frequency * M_PI / m_sampleRate;
		m_b1 = 1.0f - m_a0;
	};
	float process(const float sample)
	{
		return m_sampleLast = m_a0 * sample + m_b1 * m_sampleLast;
	};

private:
	float m_sampleRate = 48000.0f;
	float m_sampleLast = 0.0f;
	float m_a0 = 1.0f;
	float m_b1 = 0.0f;
};

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
	static const float MAXIMUM_DELAY_TIME;
	static const int DELAY_LINES_COUNT = 128;

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

	//==============================================================================

	struct Point3D {
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
	};

	Point3D substruct(Point3D a, Point3D b)
	{
		Point3D tmp;
		tmp.x = a.x - b.x;
		tmp.y = a.y - b.y;
		tmp.z = a.z - b.z;

		return tmp;
	}

	Point3D devide(Point3D a, double b)
	{
		Point3D tmp;
		tmp.x = a.x / b;
		tmp.y = a.y / b;
		tmp.z = a.z / b;

		return tmp;
	}

	Point3D sum(Point3D a, Point3D b)
	{
		Point3D tmp;
		tmp.x = a.x + b.x;
		tmp.y = a.y + b.y;
		tmp.z = a.z + b.z;

		return tmp;
	}

	Point3D multiply(Point3D a, double b)
	{
		Point3D tmp;
		tmp.x = a.x * b;
		tmp.y = a.y * b;
		tmp.z = a.z * b;

		return tmp;
	}

	double len(Point3D a)
	{
		return std::sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
	}

	Point3D normalize(Point3D a)
	{
		return devide(a, len(a));
	}


	// Generate evenly distributed points on a unit sphere using Fibonacci Sphere algorithm
	std::vector<Point3D> generateFibonacciSphere(int numPoints)
	{
		std::vector<Point3D> points;
		points.reserve(numPoints);

		// Calculate the golden angle in radians
		const double goldenAngle = M_PI * (3.0 - std::sqrt(5.0));

		for (int i = 0; i < numPoints; ++i) {
			// Calculate the y coordinate, ranging from -1 to 1
			double y = 1.0 - (2.0 * i) / (numPoints - 1); // linearly spaced from 1 to -1
			double radius = std::sqrt(1.0 - y * y); // radius of the circle at height y

			// Azimuthal angle around the y-axis
			double theta = i * goldenAngle;

			// Convert spherical coordinates to Cartesian coordinates
			double x = radius * std::cos(theta);
			double z = radius * std::sin(theta);

			points.push_back({ x, y, z });
		}

		return points;
	}
	
	//==============================================================================
	// Get point on the box
	Point3D GetIntersection(Point3D outerPoint, Point3D insidePoint, Point3D mins, Point3D maxs)
	{
		Point3D d = normalize(substruct(outerPoint, insidePoint));

		double txmax = (outerPoint.x - maxs.x) / d.x;
		double txmin = (outerPoint.x - mins.x) / d.x;

		double tymax = (outerPoint.y - maxs.y) / d.y;
		double tymin = (outerPoint.y - mins.y) / d.y;

		double tzmax = (outerPoint.z - maxs.z) / d.z;
		double tzmin = (outerPoint.z - mins.z) / d.z;

		double tx = std::min(txmax, txmin);
		double ty = std::min(tymax, tymin);
		double tz = std::min(tzmax, tzmin);

		double t = std::max(tx, std::max(ty, tz));

		return substruct(outerPoint, multiply(d, t));
	}
	
	//==============================================================================
	struct Params
	{
		// Default constructor
		Params()
			: L(0.0f), W(0.0f), H(0.0f), resonance(0.0f), damping(0.0f),
			color(0.0f), width(0.0f), mix(0.0f), volume(0.0f) // Initialize all member variables to default values
		{}

		// Parameterized constructor
		Params(float l, float w, float h, float res, float damp,
			float col, float wdt, float mx, float vol)
			: L(l), W(w), H(h), resonance(res), damping(damp),
			color(col), width(wdt), mix(mx), volume(vol)
		{}

		float L, W, H;         // Dimensions
		float resonance;       // Resonance parameter
		float damping;         // Damping parameter
		float color;           // Color parameter
		float width;           // Width parameter
		float mix;             // Mix parameter
		float volume;          // Volume parameter
	};
	
	Params m_Params;

	std::atomic<float>* LParameter = nullptr;
	std::atomic<float>* WParameter = nullptr;
	std::atomic<float>* HParameter = nullptr;
	std::atomic<float>* resonanceParameter = nullptr;
	std::atomic<float>* dampingParameter = nullptr;
	std::atomic<float>* colorParameter = nullptr;
	std::atomic<float>* widthParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	CircularBuffer m_buffer[2][DELAY_LINES_COUNT];
	OnePoleFilter m_filter[2][DELAY_LINES_COUNT];
	BiquadFilter m_lowShelf[2];
	BiquadFilter m_highShelf[2];
	float m_tmp[DELAY_LINES_COUNT];
	float m_bGain[2][DELAY_LINES_COUNT];
	float m_cGain[2][DELAY_LINES_COUNT];

	// More reflective
	const int m_primeNumbers[DELAY_LINES_COUNT] = { 631, 7, 839, 83, 29, 757, 23, 887, 31, 941, 211, 719, 857, 569, 47, 991 };
	// More smooth
	//const int m_primeNumbers[DELAY_LINES_COUNT] = { 631, 443, 509, 83, 29, 757, 23, 809, 337, 157, 211, 719, 857, 569, 47, 991 };
	//const int m_primeNumbers[DELAY_LINES_COUNT] = { 631, 443, 509, 83, 39, 757, 23, 809, 337, 157, 211, 719, 857, 569, 47, 991 };

	float m_delayTimes[DELAY_LINES_COUNT];

	//==============================================================================
	void OnParamsChanged(Params& params)
	{
		double sampleRate = getSampleRate();
		
		// Set listenets for left and right channels
		Point3D listener[2];

		double offsetY = (double)(params.W * params.width);

		listener[0].x = 0.5;
		listener[0].y = 0.5 - offsetY;
		listener[0].z = 0.5;

		listener[1].x = 0.5;
		listener[1].y = 0.5 + offsetY;
		listener[1].z = 0.5;

		Point3D mins, maxs;
		mins.x = -params.L;
		mins.y = -params.W;
		mins.z = -params.H;

		maxs.x = params.L;
		maxs.y = params.W;
		maxs.z = params.H;

		std::vector<Point3D> fibonacciSphere = generateFibonacciSphere(DELAY_LINES_COUNT);

		// TODO: Remove this hack
		for (int i = 0; i < fibonacciSphere.size(); i++)
		{
			fibonacciSphere[i].x = fibonacciSphere[i].x * 100.0;
			fibonacciSphere[i].y = fibonacciSphere[i].y * 100.0;
			fibonacciSphere[i].z = fibonacciSphere[i].z * 100.0;
		}

		for (int channel = 0; channel < 2; channel++)
		{
			for (int delayLine = 0; delayLine < DELAY_LINES_COUNT; delayLine++)
			{
				Point3D p = GetIntersection(fibonacciSphere[delayLine], listener[channel], mins, maxs);
				const float distance = 2.0f * (float)(len(p));

				// Set delay time
				float samplesCount = distance * sampleRate / 343.0f;
				m_buffer[channel][delayLine].setSize((int)samplesCount);

				//Set pre delay line gain
				constexpr auto a = 10.0f;
				m_bGain[channel][delayLine] = a / (distance + a);

				// Set feedback line LP filter
				const auto frequency = 16000.0f * (1.0 - params.damping);
				m_filter[channel][delayLine].set(frequency);
			}
		}
	}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNReverbAudioProcessor)
};
