/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string RoomReverbAudioProcessor::paramsNames[] = {		"ER Predelay",	"ER Time",		"ER Att",		"ER Damp",		"ER Width", 
																	"LR Predelay",	"LR Time",		"LR Size",   	"LR Damping",	"LR Width",
																	"Seed 1",		"Seed 2",		"Time Min",		"Complexity",
																	"ER Volume",	"LR Volume",	"Mix",			"Volume" };

const std::string RoomReverbAudioProcessor::paramsUnitNames[] = {	" ms",			" ms",			" dB",			"",				"",
																	" ms",			"",				"",				"",				"",
																	"",				"",				"",				"",
																	" dB",			" dB",			" %",			" dB" };
//==============================================================================
RoomReverbAudioProcessor::RoomReverbAudioProcessor()
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
	ERPredelayParameter      = apvts.getRawParameterValue(paramsNames[0]);
	ERTimeParameter          = apvts.getRawParameterValue(paramsNames[1]);
	ERAttenuationParameter	 = apvts.getRawParameterValue(paramsNames[2]);
	ERDampingParameter       = apvts.getRawParameterValue(paramsNames[3]);
	ERWidthParameter         = apvts.getRawParameterValue(paramsNames[4]);

	predelayParameter        = apvts.getRawParameterValue(paramsNames[5]);
	timeParameter            = apvts.getRawParameterValue(paramsNames[6]);
	resonanceParameter       = apvts.getRawParameterValue(paramsNames[7]);
	dampingParameter         = apvts.getRawParameterValue(paramsNames[8]);
	widthParameter           = apvts.getRawParameterValue(paramsNames[9]);

	combFilterSeedParameter  = apvts.getRawParameterValue(paramsNames[10]);
	allPassSeedParameter     = apvts.getRawParameterValue(paramsNames[11]);
	timeMinParameter         = apvts.getRawParameterValue(paramsNames[12]);
	complexityParameter      = apvts.getRawParameterValue(paramsNames[13]);

	ERVolumeParameter        = apvts.getRawParameterValue(paramsNames[14]);
	LRVolumeParameter        = apvts.getRawParameterValue(paramsNames[15]);
	mixParameter             = apvts.getRawParameterValue(paramsNames[16]);
	volumeParameter          = apvts.getRawParameterValue(paramsNames[17]);
}

RoomReverbAudioProcessor::~RoomReverbAudioProcessor()
{
}

//==============================================================================
const juce::String RoomReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RoomReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RoomReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RoomReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RoomReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RoomReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RoomReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RoomReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RoomReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void RoomReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RoomReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{	
	const int sr = (int)sampleRate;

	m_circularCombFilter[0].init(MAX_COMPLEXITY, 0, sr);
	m_circularCombFilter[1].init(MAX_COMPLEXITY, 1, sr);;

	const int predelaySize = (int)((float)(MAX_ER_PREDELAY_TIME + MAX_ER_TIME) * 0.001f * (float)sampleRate);

	m_earlyReflaction[0].init(predelaySize, sr, 0);
	m_earlyReflaction[1].init(predelaySize, sr, 1);
}

void RoomReverbAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RoomReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void RoomReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto ERPredelay = ERPredelayParameter->load();
	const auto ERTime = ERTimeParameter->load();
	const auto ERAttenuationdB = ERAttenuationParameter->load();
	const auto ERDamping = ERDampingParameter->load();
	const auto ERWidth = ERWidthParameter->load();
	const auto predelay = predelayParameter->load();

	CircularCombFilterAdvancedParams circularCombFilterParams;

	circularCombFilterParams.combFilterTime = 0.02f + 0.98f * timeParameter->load();
	circularCombFilterParams.combFilterResonance = 0.4f - (0.2f * (1.0f - circularCombFilterParams.combFilterTime)) + 0.45f * resonanceParameter->load();
	circularCombFilterParams.allPassTime = circularCombFilterParams.combFilterTime;
	circularCombFilterParams.allPassResonance = 0.6f * circularCombFilterParams.combFilterResonance;
	circularCombFilterParams.width = widthParameter->load();
	circularCombFilterParams.damping = 0.75f * dampingParameter->load();
	circularCombFilterParams.combFilterSeed =  (long)(combFilterSeedParameter->load());
	circularCombFilterParams.allPassSeed = (long)(allPassSeedParameter->load());
	circularCombFilterParams.timeMin = timeMinParameter->load();
	circularCombFilterParams.complexity = (int)(complexityParameter->load());
	
	const auto ERGain = juce::Decibels::decibelsToGain(ERVolumeParameter->load());
	const auto LRGain = juce::Decibels::decibelsToGain(LRVolumeParameter->load());
	const auto wet = 0.01f * mixParameter->load();
	const auto gain = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();
	const auto sampleRate = (float)getSampleRate();
	const auto sampleRateMS = 0.001f * sampleRate;
	const int predelaySamples = (int)(sampleRateMS * predelay);
	const auto dry = 1.0f - wet;

	const int ERPredelaySize = (int)(ERPredelay * sampleRateMS);
	const int ERSize = (int)(ERTime * sampleRateMS);
		
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// Early reflections setup
		auto& earlyReflection = m_earlyReflaction[channel];

		earlyReflection.set(ERDamping, ERPredelaySize, ERSize, ERWidth, ERAttenuationdB);

		// Circular comb filter setup
		auto& circularCombFilter = m_circularCombFilter[channel];
		circularCombFilter.set(circularCombFilterParams);

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];
			const float inPreDelay = earlyReflection.readDelay(predelaySamples);
			
			const float out = ERGain * earlyReflection.process(in) + LRGain * circularCombFilter.process(inPreDelay);

			channelBuffer[sample] = dry * in + wet * out;
		}
	}

	buffer.applyGain(gain);
}

//==============================================================================
bool RoomReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RoomReverbAudioProcessor::createEditor()
{
    return new RoomReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void RoomReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void RoomReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout RoomReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0],  paramsNames[0],  NormalisableRange<float>((float)MIN_ER_PREDELAY_TIME, (float)MAX_ER_PREDELAY_TIME, 1.0f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1],  paramsNames[1],  NormalisableRange<float>(		   (float)MIN_ER_TIME,          (float)MAX_ER_TIME, 1.0f, 1.0f), 40.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2],  paramsNames[2],  NormalisableRange<float>( -40.0f,  0.0f, 1.0f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3],  paramsNames[3],  NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4],  paramsNames[4],  NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>((float)MIN_ER_PREDELAY_TIME,  (float)(MAX_ER_PREDELAY_TIME + MAX_ER_TIME), 1.00f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.5f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>(   0.0f, (float)0x7fff, 1.00f), (float)(0x7fff * 0.25f)));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>(   0.0f, (float)0x7fff, 1.00f), (float)(0x7fff * 0.5f)));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[13], paramsNames[13], NormalisableRange<float>(   1.0f, (float)MAX_COMPLEXITY, 1.00f, 1.0f), 8.0f));


	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[14], paramsNames[14], NormalisableRange<float>( -60.0f,   0.0f, 1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[15], paramsNames[15], NormalisableRange<float>( -60.0f,   0.0f, 1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[16], paramsNames[16], NormalisableRange<float>(   0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[17], paramsNames[17], NormalisableRange<float>( -18.0f,  18.0f, 1.0f, 1.0f),  0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RoomReverbAudioProcessor();
}
