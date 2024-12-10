/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string CompressorAudioProcessor::paramsNames[] = { "Gain", "Attack", "Release", "Ratio", "Mix", "Volume" };

//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
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
	gainParameter    = apvts.getRawParameterValue(paramsNames[0]);
	attackParameter  = apvts.getRawParameterValue(paramsNames[1]);
	releaseParameter = apvts.getRawParameterValue(paramsNames[2]);
	ratioParameter   = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter     = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter  = apvts.getRawParameterValue(paramsNames[5]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LOG"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LIN"));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("VCA"));
	button4Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Opto"));
	button5Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Peak"));
	button6Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("RMS"));
	button7Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Slew"));
	button8Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Dual"));
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const juce::String CompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorAudioProcessor::getTailLengthSeconds() const
{
    return 1.0f;
}

int CompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = int(sampleRate);

	m_compressor[0].init(sr);
	m_compressor[1].init(sr);
	m_slewCompressor[0].init(sr);
	m_slewCompressor[1].init(sr);
	m_optoCompressor[0].init(sr);
	m_optoCompressor[1].init(sr);
	m_dualCompressor[0].init(sr);
	m_dualCompressor[1].init(sr);
}

void CompressorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto gain = juce::Decibels::decibelsToGain(gainParameter->load());
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();
	const auto ratio = ratioParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();
	const auto button4 = button4Parameter->get();
	const auto button5 = button5Parameter->get();
	const auto button6 = button6Parameter->get();
	const auto button7 = button7Parameter->get();
	const auto button8 = button8Parameter->get();

	// Mics constants
	const auto mixInverse = 1.0f - mix;
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		// VCA
		if (button3)
		{
			auto& compressor = m_compressor[channel];
			compressor.set(-20.0f, ratio, 0.0f, attack, release);

			// LOG
			if (button1)
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
			// LIN
			else
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
		}
		// Opto
		else if (button4)
		{
			auto& compressor = m_optoCompressor[channel];
			compressor.set(-20.0f, ratio, 0.0f, attack, release);

			// LOG
			if (button1)
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
			// LIN
			else
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
		}
		// Slew
		else if (button7)
		{
			auto& compressor = m_slewCompressor[channel];
			compressor.set(-20.0f, ratio, 0.0f, attack, release);

			// LOG
			if (button1)
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
			// LIN
			else
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
		}
		// Dual
		else
		{
			auto& compressor = m_dualCompressor[channel];
			compressor.set(-20.0f, ratio, 0.0f, attack, release);

			// LOG
			if (button1)
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLogRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
			// LIN
			else
			{
				// Peak
				if (button5)
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinPeak(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
				// RMS
				else
				{
					for (int sample = 0; sample < samples; sample++)
					{
						const float in = channelBuffer[sample];
						const float	out = compressor.processHardKneeLinRMS(in * gain);

						channelBuffer[sample] = volume * (mixInverse * in + mix * out);
					}
				}
			}
		}
	}
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new CompressorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout CompressorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(   0.1f, 200.0f, 0.1f,  0.4f),   10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   5.0f, 600.0f, 0.1f,  0.4f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   1.5f,   8.0f, 0.5f,  1.0f),   4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(   0.0f,   1.0f, 0.01f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>( -18.0f,  18.0f, 0.1f,  1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("LOG", "LOG", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("LIN", "LIN", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("VCA", "VCA", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("Opto", "Opto", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Peak", "Peak", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("RMS", "RMS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Slew", "Slew", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("Dual", "Dual", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}
