/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <immintrin.h>

//==============================================================================

const std::string AmbientDelayAudioProcessor::paramsNames[] =	{	"Distance 1",	"Pan 1",		"Distance 2",		"Pan 2",			"Distance 3",		"Pan 3",		"Distance 4",	"Pan 4",
																	"Attack Min",	"Attack Max",	"Attack Time Min",	"Attack Time Max",	"Sustain Min",		"Sustain Max",	"Clip Min",		"Clip Max",
																	"LP Freq Min",	"LP Freq Max",	"HP Freq Min",		"HP Freq Max",		"Distance Factor",	"Mix",			"Volume"};
const std::string AmbientDelayAudioProcessor::paramsUnitNames[] = { " m",			"",				" m",				"",					" m",				"",				" m",			"",
																	"",				"",				" ms",				" ms",				"",					"",				" dB",			" dB",
																	" Hz",			" Hz",			" Hz",				" Hz",				"",					" %",			" dB"};

//==============================================================================
AmbientDelayAudioProcessor::AmbientDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	 distance1Parameter = apvts.getRawParameterValue(paramsNames[0]);
	 pan1Parameter = apvts.getRawParameterValue(paramsNames[1]);
	 distance2Parameter = apvts.getRawParameterValue(paramsNames[2]);
	 pan2Parameter = apvts.getRawParameterValue(paramsNames[3]);
	 distance3Parameter = apvts.getRawParameterValue(paramsNames[4]);
	 pan3Parameter = apvts.getRawParameterValue(paramsNames[5]);
	 distance4Parameter = apvts.getRawParameterValue(paramsNames[6]);
	 pan4Parameter = apvts.getRawParameterValue(paramsNames[7]);
	 attackMinParameter = apvts.getRawParameterValue(paramsNames[8]);
	 attackMaxParameter = apvts.getRawParameterValue(paramsNames[9]);
	 attackTimeMinParameter = apvts.getRawParameterValue(paramsNames[10]);
	 attackTimeMaxParameter = apvts.getRawParameterValue(paramsNames[11]);
	 sustainMinParameter = apvts.getRawParameterValue(paramsNames[12]);
	 sustainMaxParameter = apvts.getRawParameterValue(paramsNames[13]);
	 clipMinParameter = apvts.getRawParameterValue(paramsNames[14]);
	 clipMaxParameter = apvts.getRawParameterValue(paramsNames[15]);
	 LPFreqMinParameter = apvts.getRawParameterValue(paramsNames[16]);
	 LPFreqMaxParameter = apvts.getRawParameterValue(paramsNames[17]);
	 HPFreqMinParameter = apvts.getRawParameterValue(paramsNames[18]);
	 HPFreqMaxParameter = apvts.getRawParameterValue(paramsNames[19]);
	 distanceFactorParameter = apvts.getRawParameterValue(paramsNames[20]);
	 mixParameter = apvts.getRawParameterValue(paramsNames[21]);
	 volumeParameter = apvts.getRawParameterValue(paramsNames[22]);
}

AmbientDelayAudioProcessor::~AmbientDelayAudioProcessor()
{
}

//==============================================================================
const juce::String AmbientDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmbientDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AmbientDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AmbientDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AmbientDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbientDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AmbientDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbientDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AmbientDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmbientDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AmbientDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int size = (int)((float)sampleRate * (float)MAXIMUM_DISTANCE / SPEED_OF_SOUND);
	const int sr = (int)sampleRate;
	
	for (auto delayLine = 0; delayLine < N_DELAY_LINES; delayLine++)
	{
		m_ambientDelay[delayLine].init(size, sr);
	}
}

void AmbientDelayAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmbientDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AmbientDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	float distances[N_DELAY_LINES];
	float pan[N_DELAY_LINES];

	distances[0] = distance1Parameter->load();
	distances[1] = distance2Parameter->load();
	distances[2] = distance3Parameter->load();
	distances[3] = distance4Parameter->load();
	pan[0] = pan1Parameter->load();
	pan[1] = pan2Parameter->load();
	pan[2] = pan3Parameter->load();
	pan[3] = pan4Parameter->load();

	float attackMin = 0.01f * attackMinParameter->load();
	float attackMax = 0.01f * attackMaxParameter->load();
	float attackTimeMin = attackTimeMinParameter->load();
	float attackTimeMax = attackTimeMaxParameter->load();
	float sustainMin = 0.01f * sustainMinParameter->load();
	float sustainMax = 0.01f * sustainMaxParameter->load();
	float clipMin = clipMinParameter->load();
	float clipMax = clipMaxParameter->load();
	float LPFreqMin = LPFreqMinParameter->load();
	float LPFreqMax = LPFreqMaxParameter->load();
	float HPFreqMin = HPFreqMinParameter->load();
	float HPFreqMax = HPFreqMaxParameter->load();
	float distanceFactor = distanceFactorParameter->load();
	float mix = 0.01f * mixParameter->load();
	float volume = juce::Decibels::decibelsToGain(volumeParameter->load());
		
	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	const float mixInverse = 1.0f - mix;
	float distanceAttenuation[N_DELAY_LINES];

	// Set delay lines
	for (auto delayLine = 0; delayLine < N_DELAY_LINES; delayLine++)
	{
		const float distance = distances[delayLine];
		const float time = distance / SPEED_OF_SOUND;

		float size = time * sampleRate;
		float lowPassFrequency = Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, LPFreqMax, LPFreqMin);
		float highPassFrequency = Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, HPFreqMin, HPFreqMax);
		float attackGain = Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, attackMin, attackMax);
		float sustainGain = Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, sustainMin, sustainMax);
		float attacktTime = Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, attackTimeMin, attackTimeMax);
		float clipGain = juce::Decibels::decibelsToGain(Helpers::Remap(distance, 0.0f, (float)MAXIMUM_DISTANCE, clipMin, clipMax));

		m_ambientDelay[delayLine].set(size, lowPassFrequency, highPassFrequency, attackGain, sustainGain, attacktTime, clipGain);

		distanceAttenuation[delayLine] = Helpers::GetDistanceAttenuation(distance, distanceFactor);
	}

	// Channel pointer
	auto* leftChannelBuffer = buffer.getWritePointer(0);
	auto* rightChannelBuffer = buffer.getWritePointer(1);

	for (int sample = 0; sample < samples; sample++)
	{
		// Read
		const float inLeft = leftChannelBuffer[sample];
		const float inRight = rightChannelBuffer[sample];
		const float monoMix = inLeft + inRight;

		// Handle delay lines
		float leftSum = 0.0f;
		float rightSum = 0.0f;

		for (auto delayLine = 0; delayLine < N_DELAY_LINES; delayLine++)
		{
			const float out = distanceAttenuation[delayLine] * m_ambientDelay[delayLine].process(monoMix);

			float left = 0.0f;
			float right = 0.0f;
			Helpers::PanMonoToStereo(out, pan[delayLine], left, right);

			leftSum += left;
			rightSum += right;
		}

		leftSum *= 0.25f;
		rightSum *= 0.25f;

		// Write to output
		const float outLeft =  mixInverse * inLeft  + mix * leftSum;
		const float outRight = mixInverse * inRight + mix * rightSum;

		leftChannelBuffer[sample] = outLeft;
		rightChannelBuffer[sample] = outRight;
	}

	buffer.applyGain(volume);
}

//==============================================================================
bool AmbientDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AmbientDelayAudioProcessor::createEditor()
{
    return new AmbientDelayAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void AmbientDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void AmbientDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AmbientDelayAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0],   NormalisableRange<float>(1.0f, (float)MAXIMUM_DISTANCE, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1],   NormalisableRange<float>(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2],   NormalisableRange<float>(1.0f, (float)MAXIMUM_DISTANCE, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3],   NormalisableRange<float>(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4],   NormalisableRange<float>(1.0f, (float)MAXIMUM_DISTANCE, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5],   NormalisableRange<float>(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6],   NormalisableRange<float>(1.0f, (float)MAXIMUM_DISTANCE, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7],   NormalisableRange<float>(-1.0f, 1.0f, 0.1f, 1.0f), 0.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8],   NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9],   NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(0.0f, 50.0f, 1.0f, 1.0f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>(0.0f, 50.0f, 1.0f, 1.0f), 5.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[13], paramsNames[13], NormalisableRange<float>(-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[14], paramsNames[14], NormalisableRange<float>(-18.0f, 18.0f, 1.0f, 1.0f), 18.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[15], paramsNames[15], NormalisableRange<float>(-18.0f, 18.0f, 1.0f, 1.0f), 18.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[16], paramsNames[16], NormalisableRange<float>(40.0f, 16000.0f, 1.0f, 0.4f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[17], paramsNames[17], NormalisableRange<float>(40.0f, 16000.0f, 1.0f, 0.4f), 10000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[18], paramsNames[18], NormalisableRange<float>(40.0f, 16000.0f, 1.0f, 0.4f), 300.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[19], paramsNames[19], NormalisableRange<float>(40.0f, 16000.0f, 1.0f, 0.4f), 300.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[20], paramsNames[20], NormalisableRange<float>(1.0f, 100.0f, 1.0f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[21], paramsNames[21], NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[22], paramsNames[22], NormalisableRange<float>(-18.0f, 18.0f, 1.0f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmbientDelayAudioProcessor();
}
