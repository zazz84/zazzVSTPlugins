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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string SpectrumMatchFFTAudioProcessor::paramsNames[] = { "LP", "HP", "Shift", "Resolution", "Ammount", "Volume" };
const std::string SpectrumMatchFFTAudioProcessor::labelNames[] = { "LP", "HP", "Shift", "Resolution", "Ammount", "Volume" };
const std::string SpectrumMatchFFTAudioProcessor::paramsUnitNames[] = { " Hz", "Hz", "", " st", " %", " dB" };

//==============================================================================
SpectrumMatchFFTAudioProcessor::SpectrumMatchFFTAudioProcessor()
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

	m_sourceSpectrumButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Learn Source"));
	m_targetSpectrumButton = static_cast<juce::AudioParameterBool*>(apvts.getParameter("Learn Target"));
}

SpectrumMatchFFTAudioProcessor::~SpectrumMatchFFTAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumMatchFFTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumMatchFFTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumMatchFFTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumMatchFFTAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumMatchFFTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumMatchFFTAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumMatchFFTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumMatchFFTAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumMatchFFTAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumMatchFFTAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumMatchFFTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	setLatencySamples(m_spectrumDetect[0].getLatencyInSamples());

	const int sr = (int)sampleRate;

	m_spectrumDetect[0].reset();
	m_spectrumDetect[1].reset();

	m_spectrumDetectTarget[0].reset();
	m_spectrumDetectTarget[1].reset();

	m_spectrumApply[0].reset();
	m_spectrumApply[1].reset();

	m_spectrumApply[0].init(sr);
	m_spectrumApply[1].init(sr);

	loadSourceSpectrum();
	loadTargetSpectrum();

	// Get all parameters
	for (int i = 0; i < Parameters::COUNT; i++)
	{
		m_parametersValues[i] = m_parameters[i]->load();
	}

	// Calculate runtime spectrums
	if (!m_sourceSpectrum.empty())
	{
		updateSourceSpectrum(m_parametersValues[Parameters::FrequencyShift], m_parametersValues[Parameters::Resolution]);
	}
	if (!m_targetSpectrum.empty())
	{
		updateTargetSpectrum(m_parametersValues[Parameters::Resolution]);
	}
	if (!m_sourceSpectrumRuntime.empty() && !m_targetSpectrumRuntime.empty())
	{
		updateFilterSpectrum(m_parametersValues[Parameters::Ammount], m_parametersValues[Parameters::HighPassFilter], m_parametersValues[Parameters::LowPassFilter]);
	}
}

void SpectrumMatchFFTAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumMatchFFTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectrumMatchFFTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

    // Get all parameters
    std::array<float, Parameters::COUNT> parametersValues;
    for (int i = 0; i < Parameters::COUNT; i++)
    {
        parametersValues[i] = m_parameters[i]->load();
    }
	
	const auto gain = juce::Decibels::decibelsToGain(parametersValues[Parameters::Volume]);

	// Mics constants
	const auto channels = getTotalNumOutputChannels();
	const auto samples = buffer.getNumSamples();
	bool sourceSpectrumCahnged = false;
	bool targetSpectrumChanged = false;

	//==============================================================================
	// Learn source spectrum
	const bool learnSource = m_sourceSpectrumButton->get();
	if (learnSource)
	{
		for (int channel = 0; channel < channels; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& spectrumDetect = m_spectrumDetect[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				spectrumDetect.processSample(channelBuffer[sample], false);
			}
		}
	}

	if (learnSource == false && m_learnSourceLast == true)
	{
		storeSourceSpectrum();
		loadSourceSpectrum();

		sourceSpectrumCahnged = true;
	}

	m_learnSourceLast = learnSource;
		
	//==============================================================================
	// Learn target spectrum
	const bool learnTarget = m_targetSpectrumButton->get();

	if (learnTarget)
	{
		for (int channel = 0; channel < channels; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& spectrumDetect = m_spectrumDetectTarget[channel];

			for (int sample = 0; sample < samples; sample++)
			{
				spectrumDetect.processSample(channelBuffer[sample], false);
			}
		}
	}

	if (learnTarget == false && m_learnTargetLast == true)
	{
		storeTargetSpectrum();
		loadTargetSpectrum();

		targetSpectrumChanged = true;
	}

	m_learnTargetLast = learnTarget;

	//==============================================================================
	// Prepare runtime spectrums
	if (!m_sourceSpectrum.empty() &&
		(parametersValues[Parameters::FrequencyShift] != m_parametersValues[Parameters::FrequencyShift] ||
		parametersValues[Parameters::Resolution] != m_parametersValues[Parameters::Resolution]) ||
		sourceSpectrumCahnged)
	{
		updateSourceSpectrum(parametersValues[Parameters::FrequencyShift], parametersValues[Parameters::Resolution]);
		
		sourceSpectrumCahnged = true;
	}

	if (!m_targetSpectrum.empty() &&
		(parametersValues[Parameters::Resolution] != m_parametersValues[Parameters::Resolution] ||
		targetSpectrumChanged))
	{
		updateTargetSpectrum(parametersValues[Parameters::Resolution]);

		targetSpectrumChanged = true;
	}

	if (!m_sourceSpectrumRuntime.empty() &&
		!m_targetSpectrumRuntime.empty() &&
		(parametersValues[Parameters::Ammount] != m_parametersValues[Parameters::Ammount] ||
		parametersValues[Parameters::HighPassFilter] != m_parametersValues[Parameters::HighPassFilter] ||
		parametersValues[Parameters::LowPassFilter] != m_parametersValues[Parameters::LowPassFilter] ||
		sourceSpectrumCahnged ||
		targetSpectrumChanged))
	{
		updateFilterSpectrum(parametersValues[Parameters::Ammount], parametersValues[Parameters::HighPassFilter], parametersValues[Parameters::LowPassFilter]);
	}

	//==============================================================================
	// Apply filter
	if (!m_filterSpectrumRuntime.empty())
	{
		for (int channel = 0; channel < channels; channel++)
		{
			// Channel pointer
			auto* channelBuffer = buffer.getWritePointer(channel);
			auto& spectrumApply = m_spectrumApply[channel];

			spectrumApply.set(m_filterSpectrumRuntime);

			for (int sample = 0; sample < samples; sample++)
			{
				channelBuffer[sample] = spectrumApply.processSample(channelBuffer[sample], false);
			}
		}
	}

	buffer.applyGain(gain);

	// Store parameters
	std::copy(std::begin(parametersValues), std::end(parametersValues), std::begin(m_parametersValues));
}

//==============================================================================
bool SpectrumMatchFFTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumMatchFFTAudioProcessor::createEditor()
{
    return new SpectrumMatchFFTAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void SpectrumMatchFFTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void SpectrumMatchFFTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SpectrumMatchFFTAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::HighPassFilter], paramsNames[Parameters::HighPassFilter], NormalisableRange<float>( 20.0f, 660.0f, 1.0f, 0.45f), 20.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::LowPassFilter], paramsNames[Parameters::LowPassFilter], NormalisableRange<float>( 660.0f, 20000.0f, 1.0f, 0.45f), 20000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::FrequencyShift], paramsNames[Parameters::FrequencyShift], NormalisableRange<float>( 0.25f, 4.0f,  0.01f, 0.45f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Resolution], paramsNames[Parameters::Resolution], NormalisableRange<float>( 1.0f, 12.0f, 1.0f, 1.0f), 1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Ammount], paramsNames[Parameters::Ammount], NormalisableRange<float>( -200.0f, 200.0f,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[Parameters::Volume], paramsNames[Parameters::Volume], NormalisableRange<float>( -18.0f,  18.0f,  0.1f, 1.0f),  0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("Learn Source", "Learn Source", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("Learn Target", "Learn Target", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumMatchFFTAudioProcessor();
}
