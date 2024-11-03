/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string SineWaveshaperAudioProcessor::paramsNames[] = { "Gain", "Shape", "Spread", "Mix", "Volume" };

//==============================================================================
SineWaveshaperAudioProcessor::SineWaveshaperAudioProcessor()
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
	gainParameter   = apvts.getRawParameterValue(paramsNames[0]);
	shapeParameter  = apvts.getRawParameterValue(paramsNames[1]);
	spreadParameter = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter    = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter = apvts.getRawParameterValue(paramsNames[4]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("1"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("2"));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("32x"));
}

SineWaveshaperAudioProcessor::~SineWaveshaperAudioProcessor()
{
}

//==============================================================================
const juce::String SineWaveshaperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SineWaveshaperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SineWaveshaperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SineWaveshaperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SineWaveshaperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SineWaveshaperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SineWaveshaperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SineWaveshaperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SineWaveshaperAudioProcessor::getProgramName (int index)
{
    return {};
}

void SineWaveshaperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SineWaveshaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;
	const int oversampleRate = OVERSAMPLE * sr;

	m_DCFilter[0].init(sr);
	m_DCFilter[1].init(sr);

	m_DCFilter[0].setHighPass(15.0f, 0.7f);
	m_DCFilter[1].setHighPass(15.0f, 0.7f);

	m_oversampleFilter[0].init(oversampleRate);
	m_oversampleFilter[1].init(oversampleRate);

	m_downsampleFilter[0].init(oversampleRate);
	m_downsampleFilter[1].init(oversampleRate);

	const float f = fminf(30000.0f, 0.5f * (float)sampleRate);
	const float q = 0.75f;
	m_oversampleFilter[0].setLowPass(f, q);
	m_oversampleFilter[1].setLowPass(f, q);

	m_downsampleFilter[0].setLowPass(f, q);
	m_downsampleFilter[1].setLowPass(f, q);
}

void SineWaveshaperAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SineWaveshaperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SineWaveshaperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto gain = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto shape = shapeParameter->load();
	const auto spread = spreadParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	// Get buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();

	// Mics constants
	const auto mixInverse = 1.0f - mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	const auto gainReduction = juce::Decibels::decibelsToGain(-30.0f);

	if (button3)
	{
		for (int channel = 0; channel < channels; channel++)
		{
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& DCFilter = m_DCFilter[channel];
			auto& oversampleFilter = m_oversampleFilter[channel];
			auto& downsampleFilter = m_downsampleFilter[channel];

			float offset = (channel == 0) ? -spread : spread;
			
			if (button1)
			{
				auto& waveShaper = m_inflatorWaveShaper[channel];
				waveShaper.set(shape);

				for (int sample = 0; sample < samples; sample++)
				{
					const float in = channelBuffer[sample];

					m_temp[0] = oversampleFilter.processDF1((float)OVERSAMPLE * gain * in);

					for (int i = 1; i < OVERSAMPLE; i++)
					{
						m_temp[i] = oversampleFilter.processDF1(0.0f);
					}
					
					float sum = 0.0f;
					for (int i = 0; i < OVERSAMPLE; i++)
					{
						float tmp = m_temp[i];
						tmp = waveShaper.process(tmp + offset);
						tmp = downsampleFilter.processDF1(tmp);
						sum += tmp;
					}

					const float out = DCFilter.processDF1(gainReduction * sum);

					channelBuffer[sample] = volume * (mixInverse * in + mix * out);
				}
			}
			else
			{
				auto& waveShaper = m_sineWaveShaper[channel];
				waveShaper.set(shape);

				for (int sample = 0; sample < samples; sample++)
				{
					const float in = channelBuffer[sample];

					m_temp[0] = oversampleFilter.processDF1((float)OVERSAMPLE * gain * in);

					for (int i = 1; i < OVERSAMPLE; i++)
					{
						m_temp[i] = oversampleFilter.processDF1(0.0f);
					}

					float sum = 0.0f;
					for (int i = 0; i < OVERSAMPLE; i++)
					{
						float tmp = m_temp[i];
						tmp = waveShaper.process(tmp + offset);
						tmp = downsampleFilter.processDF1(tmp);
						sum += tmp;
					}

					const float out = DCFilter.processDF1(gainReduction * sum);

					channelBuffer[sample] = volume * (mixInverse * in + mix * out);
				}
			}
		}
	}
	else
	{
		for (int channel = 0; channel < channels; channel++)
		{
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& DCFilter = m_DCFilter[channel];

			float offset = (channel == 0) ? -spread : spread;

			if (button1)
			{
				auto& waveShaper = m_inflatorWaveShaper[channel];
				waveShaper.set(shape);

				for (int sample = 0; sample <  samples; sample++)
				{
					const float in = channelBuffer[sample];

					float out = waveShaper.process(gain * (in + offset));

					out = DCFilter.processDF1(out);

					channelBuffer[sample] = volume * (mixInverse * in + mix * out);
				}
			}
			else
			{
				auto& waveShaper = m_sineWaveShaper[channel];
				waveShaper.set(shape);

				for (int sample = 0; sample < samples; sample++)
				{
					const float in = channelBuffer[sample];

					float out = waveShaper.process(gain * (in + offset));

					out = DCFilter.processDF1(out);

					channelBuffer[sample] = volume * (mixInverse * in + mix * out);
				}
			}
		}
	}
}

//==============================================================================
bool SineWaveshaperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SineWaveshaperAudioProcessor::createEditor()
{
    return new SineWaveshaperAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SineWaveshaperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SineWaveshaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SineWaveshaperAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f, 18.0f, 0.1f,  1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  -1.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f,  1.0f, 0.01f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -18.0f, 18.0f, 0.1f,  1.0f), 0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("1", "1", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("2", "2", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("32x", "32x", true));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SineWaveshaperAudioProcessor();
}
