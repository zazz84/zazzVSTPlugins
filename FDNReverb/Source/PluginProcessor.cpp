/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string FDNReverbAudioProcessor::paramsNames[] = { "L", "W", "H", "Reflections", "ER Damping", "ER Width", "Predelay", "Time", "Size", "Color", "LR Damping", "LR Width", "ER Volume", "LR Volume", "Mix", "Volume" };
const std::string FDNReverbAudioProcessor::paramsUnitNames[] = { " m", " m", " m", "", " %", " %", " ms", " s", "", "", " %", " %", " dB", " dB", " %", " dB" };
const float FDNReverbAudioProcessor::MAXIMUM_DELAY_TIME = 0.08f;
const float FDNReverbAudioProcessor::AVERAGE_DELAY_TIME = 0.452648f;
const float FDNReverbAudioProcessor::MAXIMUM_ROOM_DIMENSION = 40.0f;
/*const float FDNReverbAudioProcessor::LATE_REFLECTION_DELAY_TIME_NORMALIZED[] = {0.636737f, 0.446518f, 0.513624f, 0.083762f,
																				0.029272f, 0.763866f, 0.023210f, 0.816345f,
																				0.340262f, 0.158223f, 0.212917f, 0.725045f,
																				0.865289f, 0.574166f, 0.047429f, 1.000000f };*/

const float FDNReverbAudioProcessor::LATE_REFLECTION_DELAY_TIME_NORMALIZED[] = { 0.9137f, 0.5557f, 0.1591f, 0.0544f,
																				 0.5253f, 0.8724f, 0.0168f, 0.6907f,
																				 0.6329f, 0.0941f, 0.8688f, 0.7679f,
																				 0.0428f, 0.4224f, 0.6195f, 1.0000f };

/*const float FDNReverbAudioProcessor::LATE_REFLECTION_DELAY_TIME_NORMALIZED[] = { 0.0612f, 0.0577f, 0.0971f, 0.1593f,
																				 0.1836f, 0.2519f, 0.3399f, 0.3627f,
																				 0.4628f, 0.5190f, 0.6070f, 0.7026f,
																				 0.7860f, 0.8255f, 0.9196f, 1.0000f };*/


//==============================================================================
FDNReverbAudioProcessor::FDNReverbAudioProcessor()
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
	// Early reflections
	LParameter = apvts.getRawParameterValue(paramsNames[0]);
	WParameter = apvts.getRawParameterValue(paramsNames[1]);
	HParameter = apvts.getRawParameterValue(paramsNames[2]);
	ReflectionsParameter = apvts.getRawParameterValue(paramsNames[3]);
	ERDampingParameter = apvts.getRawParameterValue(paramsNames[4]);
	ERWidthParameter = apvts.getRawParameterValue(paramsNames[5]);
	// Late reflections
	LRPredelayParameter = apvts.getRawParameterValue(paramsNames[6]);
	timeParameter = apvts.getRawParameterValue(paramsNames[7]);
	sizeParameter = apvts.getRawParameterValue(paramsNames[8]);
	colorParameter = apvts.getRawParameterValue(paramsNames[9]);
	LRDampingParameter = apvts.getRawParameterValue(paramsNames[10]);
	LRWidthParameter = apvts.getRawParameterValue(paramsNames[11]);
	// Mix
	ERVolumeParameter = apvts.getRawParameterValue(paramsNames[12]);
	LRVolumeParameter = apvts.getRawParameterValue(paramsNames[13]);
	mixParameter = apvts.getRawParameterValue(paramsNames[14]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[15]);
}

FDNReverbAudioProcessor::~FDNReverbAudioProcessor()
{
}

//==============================================================================
const juce::String FDNReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FDNReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FDNReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FDNReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FDNReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FDNReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FDNReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FDNReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FDNReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FDNReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int maximumERSize = (int)(2.0 * (double)MAXIMUM_ROOM_DIMENSION * sampleRate / SPEED_OF_SOUND);
	const float maximumLRSize = MAXIMUM_DELAY_TIME * (float)sampleRate;
	const int sr = (int)sampleRate;
	const int maximumPredelayTimeSamples = (int)((float)sampleRate * MAXIMUM_PREDELAY_MS * 0.001f);
	
	for (int channel = 0; channel < 2; channel++)
	{
		// Early reflection
		m_earlyReflections[channel].init(MAXIMUM_ROOM_DIMENSION, sr, MAXIMUM_REFLECTIONS_COUNT);

		// Late reflactions
		m_predelay[channel].init(maximumPredelayTimeSamples);
		
		// Colour filters
		m_lowShelf[channel].init(sr);
		m_highShelf[channel].init(sr);
		m_highPass[channel].init(sr);

		// Late reflections
		for (int delayLine = 0; delayLine < DELAY_LINES_COUNT; delayLine++)
		{
			m_bufferLR[channel][delayLine].init((int)(maximumLRSize * LATE_REFLECTION_DELAY_TIME_NORMALIZED[delayLine]));
			m_filterLR[channel][delayLine].init(sr);
		}
	}
}

void FDNReverbAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FDNReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FDNReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	Params params
	(
		// Early reflections
		LParameter->load(),
		WParameter->load(),
		HParameter->load(),
		ReflectionsParameter->load(),
		ERDampingParameter->load(),
		ERWidthParameter->load(),
		// Late reflections
		LRPredelayParameter->load(),
		timeParameter->load(),
		sizeParameter->load(),
		LRDampingParameter->load(),
		colorParameter->load(),
		LRWidthParameter->load(),
		// Mix
		juce::Decibels::decibelsToGain(ERVolumeParameter->load()),
		juce::Decibels::decibelsToGain(LRVolumeParameter->load()),
		mixParameter->load(),
		juce::Decibels::decibelsToGain(volumeParameter->load())
	);

	// TODO: Do not update if params did not change
	OnParamsChanged(params);

	// Mics constants
	const auto mixInverse = 1.0f - params.mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto feedback = fminf(0.97f, m_feedback);
	const auto shelfFilterGain = params.color * 4.5f;

	// Process buffer
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		auto& bufferLR = m_bufferLR[channel];
		auto& filterLF = m_filterLR[channel];
		auto& lowShelf = m_lowShelf[channel];
		auto& highShelf = m_highShelf[channel];
		auto& highPass = m_highPass[channel];
		auto& gainsLR = m_gainsLR[channel];
		auto& earlyReflections = m_earlyReflections[channel];
		auto& predelay = m_predelay[channel];

		lowShelf.setLowShelf(660.0f, 0.4f, -shelfFilterGain);
		highShelf.setHighShelf(660.0f, 0.4f, shelfFilterGain);
		highPass.setHighPass(40.0f * (2.0f + params.color), 0.6f);

		for (int sample = 0; sample < samples; sample++)
		{
			const float in = channelBuffer[sample];

			// Early reflections
			float er = earlyReflections.process(in);

			// Late reflections
			// Add color
			predelay.writeSample(in);
			const float inColorDelayed = highPass.processDF1(lowShelf.processDF1(highShelf.processDF1(predelay.read())));

			float lr = 0.0f;
			for (int i = 0; i < DELAY_LINES_COUNT; i++)
			{
				m_tmp[i] = bufferLR[i].read();

				// Filter
				m_tmp[i] = filterLF[i].process(m_tmp[i]);

				lr += m_tmp[i];
			}

			// FWHT
			FWHT(m_tmp);

			// Writte sample
			for (int i = 0; i < DELAY_LINES_COUNT; i++)
			{
				bufferLR[i].writeSample(gainsLR[i] * inColorDelayed + feedback * m_tmp[i]);
			}
			
			channelBuffer[sample] = params.volume * (mixInverse * in + params.mix * (params.ERvolume * er + params.LRvolume * lr));
		}
	}
}

//==============================================================================
bool FDNReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FDNReverbAudioProcessor::createEditor()
{
    return new FDNReverbAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void FDNReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void FDNReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout FDNReverbAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	// Early reflections
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( 1.0f, MAXIMUM_ROOM_DIMENSION, 0.1f, 1.0f), 9.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>( 1.0f, MAXIMUM_ROOM_DIMENSION, 0.1f, 1.0f), 7.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>( 1.0f, MAXIMUM_ROOM_DIMENSION, 0.1f, 1.0f), 4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>( 2.0f, MAXIMUM_REFLECTIONS_COUNT, 1.0f, 1.0f), 8.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	
	// Late reflections
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[6], paramsNames[6], NormalisableRange<float>( 0.0f, MAXIMUM_PREDELAY_MS, 0.01f, 1.0f), 10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[7], paramsNames[7], NormalisableRange<float>( 0.1f, 4.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[8], paramsNames[8], NormalisableRange<float>( 0.0f, 1.0f, 0.01f, 1.0f), 0.5f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[9], paramsNames[9], NormalisableRange<float>(-1.0f, 1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[10], paramsNames[10], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[11], paramsNames[11], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 50.0f));
	// Mix
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[12], paramsNames[12], NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[13], paramsNames[13], NormalisableRange<float>(-60.0f, 0.0f, 0.1f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[14], paramsNames[14], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[15], paramsNames[15], NormalisableRange<float>(-18.0f, 18.0f,  0.1f, 1.0f), 0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FDNReverbAudioProcessor();
}