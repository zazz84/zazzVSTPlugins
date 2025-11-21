/*
 * Copyright (C) 2025 Filip Cenzak (filip.c@centrum.cz)
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

//#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string ResynthesizerAudioProcessor::paramsNames[] = { "Frequency1", "Frequency2", "Frequency3", "Frequency4", "Frequency5", "Frequency6", "Frequency7", "Frequency8",
																 "Volume1",    "Volume2",    "Volume3",    "Volume4",    "Volume5",    "Volume6",    "Volume7",    "Volume8",
																 "Volume", "MinimumStep", "Shape", "Factor", "Style" };
const std::string ResynthesizerAudioProcessor::labelNames[] =  { "", "", "", "", "", "", "", "",
																 "", "", "", "", "", "", "", "",
																 "Volume", "MinimumStep", "Shape", "Factor", "Style" };
const std::string ResynthesizerAudioProcessor::paramsUnitNames[] = { " Hz", " Hz", " Hz", " Hz", " Hz", " Hz", " Hz", " Hz",
																	 " dB", " dB", " dB", " dB", " dB", " dB", " dB", " dB",
																	 " dB", " st", " %", "", "" };

//==============================================================================
ResynthesizerAudioProcessor::ResynthesizerAudioProcessor()
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
	for (int i = 0; i < Parameters::COUNT; i++)
    {
        m_parameters[i] = apvts.getRawParameterValue(paramsNames[i]);
    }

	m_learnButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Learn"));
}

ResynthesizerAudioProcessor::~ResynthesizerAudioProcessor()
{
}

//==============================================================================
const juce::String ResynthesizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ResynthesizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ResynthesizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ResynthesizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ResynthesizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ResynthesizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ResynthesizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ResynthesizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ResynthesizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ResynthesizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ResynthesizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	const int sr = (int)sampleRate;

	m_pitchDetection.init(sampleRate);

	for (auto& oscilator : m_oscilators)
	{
		oscilator.init(sr);
	}	
	
	for (auto& noiseFilter : m_noiseFilters)
	{
		noiseFilter.init(sr);
	}

	m_smoother.init(sr);
	m_smoother.set(2.0f);
}

void ResynthesizerAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ResynthesizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ResynthesizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all params
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }

	const bool learnButton = m_learnButton->get();

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();

	// Begin learning
	if (m_learnButtonLast == false && learnButton == true)
	{
		m_pitchDetection.reset();
		
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);

		for (int sample = 0; sample < samples; sample++)
		{
			m_pitchDetection.process(channelBuffer[sample]);
		}
	}
	// Continue learning
	else if (m_learnButtonLast == true && learnButton == true)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(0);

		for (int sample = 0; sample < samples; sample++)
		{
			m_pitchDetection.process(channelBuffer[sample]);
		}
	}
	// Stop learning
	else if (m_learnButtonLast == true && learnButton == false)
	{
		m_spectrum.clear();

		m_spectrum = m_pitchDetection.getSpectrum(N_OSCILATORS, (int)parametersValues[Parameters::MinimumStep]);

		setParameterRaw(paramsNames[Parameters::Frequency1], m_spectrum[0].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency2], m_spectrum[1].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency3], m_spectrum[2].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency4], m_spectrum[3].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency5], m_spectrum[4].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency6], m_spectrum[5].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency7], m_spectrum[6].frequency);
		setParameterRaw(paramsNames[Parameters::Frequency8], m_spectrum[7].frequency);

		setParameterRaw(paramsNames[Parameters::Volume1], juce::Decibels::gainToDecibels(m_spectrum[0].gain));
		setParameterRaw(paramsNames[Parameters::Volume2], juce::Decibels::gainToDecibels(m_spectrum[1].gain));
		setParameterRaw(paramsNames[Parameters::Volume3], juce::Decibels::gainToDecibels(m_spectrum[2].gain));
		setParameterRaw(paramsNames[Parameters::Volume4], juce::Decibels::gainToDecibels(m_spectrum[3].gain));
		setParameterRaw(paramsNames[Parameters::Volume5], juce::Decibels::gainToDecibels(m_spectrum[4].gain));
		setParameterRaw(paramsNames[Parameters::Volume6], juce::Decibels::gainToDecibels(m_spectrum[5].gain));
		setParameterRaw(paramsNames[Parameters::Volume7], juce::Decibels::gainToDecibels(m_spectrum[6].gain));
		setParameterRaw(paramsNames[Parameters::Volume8], juce::Decibels::gainToDecibels(m_spectrum[7].gain));
	}

	m_learnButtonLast = learnButton;

	// Set oscilators frequency
	const int frequencyIdx = Parameters::Frequency1;
	const int volumeIdx = Parameters::Volume1;

	if (parametersValues[Parameters::Style] == 1)
	{
		const float shape = 0.01f * parametersValues[Parameters::Shape];

		for (int channel = 0; channel < 1; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);

			for (int sample = 0; sample < samples; sample++)
			{
				// Set filters
				const float factorSmooth = m_smoother.process(parametersValues[Parameters::Factor]);

				for (int i = 0; i < N_OSCILATORS; i++)
				{
					const float frequencyClamped = Math::clamp(factorSmooth * parametersValues[frequencyIdx + i], 20.0f, 16000.0f);
					m_oscilators[i].set(frequencyClamped, shape);
				}

				float out = 0.0f;
				for (int i = 0; i < N_OSCILATORS; i++)
				{
					out += juce::Decibels::decibelsToGain(parametersValues[volumeIdx + i]) * m_oscilators[i].process();
				}

				//Out
				channelBuffer[sample] = out;
			}
		}
	}
	else if (parametersValues[Parameters::Style] == 2)
	{
		const float q = 1000.0f - 9.5f * parametersValues[Parameters::Shape];

		for (int channel = 0; channel < 1; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);

			for (int sample = 0; sample < samples; sample++)
			{
				// Set filters
				const float factorSmooth = m_smoother.process(parametersValues[Parameters::Factor]);

				for (int i = 0; i < N_OSCILATORS; i++)
				{
					const float frequencyClamped = Math::clamp(factorSmooth * parametersValues[frequencyIdx + i], 20.0f, 16000.0f);
					m_noiseFilters[i].setBandPassPeakGain(frequencyClamped, q);
				}

				float out = 0.0f;
				for (int i = 0; i < N_OSCILATORS; i++)
				{
					out += juce::Decibels::decibelsToGain(36.0f + parametersValues[volumeIdx + i]) * m_noiseFilters[i].processDF1(m_noiseGenerators[0].process());
				}

				//Out
				channelBuffer[sample] = out;
			}
		}
	}

	buffer.applyGain(juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]));
}

//==============================================================================
bool ResynthesizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ResynthesizerAudioProcessor::createEditor()
{
    //return m_audioProcessorEditor = new ResynthesizerAudioProcessorEditor (*this, apvts);
    return new ResynthesizerAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void ResynthesizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void ResynthesizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ResynthesizerAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	constexpr float FREQUENCY_MIN = 20.0f;
	constexpr float FREQUENCY_MAX = 16000.0f;
	constexpr float SLOPE = 0.4f;
	constexpr float dB = 100.0f;
	
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency1], paramsNames[Parameters::Frequency1], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency2], paramsNames[Parameters::Frequency2], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency3], paramsNames[Parameters::Frequency3], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency4], paramsNames[Parameters::Frequency4], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency5], paramsNames[Parameters::Frequency5], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency6], paramsNames[Parameters::Frequency6], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency7], paramsNames[Parameters::Frequency7], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Frequency8], paramsNames[Parameters::Frequency8], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, SLOPE), 20.0f));
	
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume1], paramsNames[Parameters::Volume1], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume2], paramsNames[Parameters::Volume2], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume3], paramsNames[Parameters::Volume3], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume4], paramsNames[Parameters::Volume4], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume5], paramsNames[Parameters::Volume5], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume6], paramsNames[Parameters::Volume6], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume7], paramsNames[Parameters::Volume7], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume8], paramsNames[Parameters::Volume8], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f), -dB));
	
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Style], paramsNames[Parameters::Style], NormalisableRange<float>( 1.0f, 2.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Factor], paramsNames[Parameters::Factor], NormalisableRange<float>( 0.1f, 10.0f, 0.001f, 0.4f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Shape], paramsNames[Parameters::Shape], NormalisableRange<float>( 0.0f, 100.0f, 1.0f, 1.0f),  0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::MinimumStep], paramsNames[Parameters::MinimumStep], NormalisableRange<float>( 0.0f, 12.0f,  1.0f, 1.0f),  1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume], paramsNames[Parameters::Volume], NormalisableRange<float>( -dB, dB,  0.1f, 1.0f),  0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("Learn", "Learn", false));

	return layout;
}

//==============================================================================
void ResynthesizerAudioProcessor::setParameterRaw(const juce::String& paramID, float rawValue)
{
	if (auto* param = apvts.getParameter(paramID))
	{
		const float normalized = param->convertTo0to1(rawValue);
		param->setValueNotifyingHost(normalized);
	}
	else
	{
		DBG("Parameter not found: " << paramID);
	}
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ResynthesizerAudioProcessor();
}
