/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const std::string BiquadFilterAudioProcessor::paramsNames[] =		{ "Frequency",	"Gain", "Q",	"Mix",	"Volume" };
const std::string BiquadFilterAudioProcessor::labelNames[] =		{ "Frequency",	"Gain", "Q",	"Mix",	"Volume" };
const std::string BiquadFilterAudioProcessor::paramsUnitNames[] =	{ " Hz",		" dB",	"",		" %",	" dB" };

//==============================================================================
BiquadFilterAudioProcessor::BiquadFilterAudioProcessor()
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
	frequencyParameter = apvts.getRawParameterValue(paramsNames[0]);
	gainParameter      = apvts.getRawParameterValue(paramsNames[1]);
	QParameter         = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);

	button1Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LP"));
	button2Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("HP"));
	button3Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("AP"));
	button4Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("BP"));
	button5Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("N"));
	button6Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("P"));
	button7Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("LS"));
	button8Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("HS"));

	button9Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF1"));
	button10Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF2"));
	button11Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF1T"));
	button12Parameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("DF2T"));
}

BiquadFilterAudioProcessor::~BiquadFilterAudioProcessor()
{
}

//==============================================================================
const juce::String BiquadFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BiquadFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BiquadFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BiquadFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BiquadFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BiquadFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BiquadFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BiquadFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BiquadFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void BiquadFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BiquadFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	int sr = (int)sampleRate;
	m_filter[0].init(sr);
	m_filter[1].init(sr);

	m_frequencySmoother.set(sr);
	m_gainSmoother.set(sr);
	m_QSmoother.set(sr);
	m_mixSmoother.set(sr);
	m_volumeSmoother.set(sr);

	const float frequency = 6.0f;
	m_frequencySmoother.set(frequency);
	m_gainSmoother.set(frequency);
	m_QSmoother.set(frequency);
	m_mixSmoother.set(frequency);
	m_volumeSmoother.set(frequency);
}

void BiquadFilterAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BiquadFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BiquadFilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Buttons
	const auto button1 = button1Parameter->get();
	const auto button2 = button2Parameter->get();
	const auto button3 = button3Parameter->get();
	const auto button4 = button4Parameter->get();
	const auto button5 = button5Parameter->get();
	const auto button6 = button6Parameter->get();
	const auto button7 = button7Parameter->get();

	const auto button9 = button9Parameter->get();
	const auto button10 = button10Parameter->get();
	const auto button11 = button11Parameter->get();

	// Get params
	const auto frequency = frequencyParameter->load();
	const auto gain = gainParameter->load();
	const auto Q = QParameter->load();
	const auto mix = 0.01f * mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	// Set filter type and algorithm
	for (int channel = 0; channel < channels; ++channel)
	{
		auto& filter = m_filter[channel];

		// Set filter type
		if (button1)
			filter.setType(BiquadFilter::Type::LowPass);
		else if (button2)
			filter.setType(BiquadFilter::Type::HighPass);
		else if (button3)
			filter.setType(BiquadFilter::Type::AllPass);
		else if (button4)
			filter.setType(BiquadFilter::Type::BandPassPeakGain);
		else if (button5)
			filter.setType(BiquadFilter::Type::Notch);
		else if (button6)
			filter.setType(BiquadFilter::Type::Peak);
		else if (button7)
			filter.setType(BiquadFilter::Type::LowShelf);
		else
			filter.setType(BiquadFilter::Type::HighShelf);

		// Set processing type
		if (button9)
			filter.setAlgorithm(BiquadFilter::Algorithm::DF1);
		else if (button10)
			filter.setAlgorithm(BiquadFilter::Algorithm::DF2);
		else if (button11)
			filter.setAlgorithm(BiquadFilter::Algorithm::DF1T);
		else
			filter.setAlgorithm(BiquadFilter::Algorithm::DF2T);
	}

	if (channels == 1)
	{
		for (int sample = 0; sample < samples; sample++)
		{
			const float frequencySmooth = m_frequencySmoother.process(frequency);
			const float gainSmooth = m_gainSmoother.process(gain);
			const float QSmooth = m_QSmoother.process(Q);
			const float mixSmooth = m_mixSmoother.process(mix);
			const float volumeSmooth = m_volumeSmoother.process(volume);

			auto* channelBuffer = buffer.getWritePointer(0);
			auto& filter = m_filter[0];

			filter.set(frequencySmooth, QSmooth, gainSmooth);

			const float in = channelBuffer[sample];
			const float inFiltered = filter.process(in);
			channelBuffer[sample] = volumeSmooth * ((1.0f - mixSmooth) * in + inFiltered * mixSmooth);
		}
	}
	else
	{
		for (int sample = 0; sample < samples; sample++)
		{
			const float frequencySmooth = m_frequencySmoother.process(frequency);
			const float gainSmooth = m_gainSmoother.process(gain);
			const float QSmooth = m_QSmoother.process(Q);
			const float mixSmooth = m_mixSmoother.process(mix);
			const float volumeSmooth = m_volumeSmoother.process(volume);

			auto* leftChannelBuffer = buffer.getWritePointer(0);
			auto* rightChannelBuffer = buffer.getWritePointer(1);
			auto& filterLeft = m_filter[0];
			auto& filterRight = m_filter[1];

			filterLeft.set(frequencySmooth, QSmooth, gainSmooth);
			filterRight.set(frequencySmooth, QSmooth, gainSmooth);

			const float inLeft = leftChannelBuffer[sample];
			const float inRight = rightChannelBuffer[sample];

			const float inLeftFiltered = filterLeft.process(inLeft);
			const float inRightFiltered = filterRight.process(inRight);

			const float dry = volumeSmooth * (1.0f - mixSmooth);
			const float wet = volumeSmooth * mixSmooth;

			leftChannelBuffer[sample] = dry * inLeft + wet * inLeftFiltered;
			rightChannelBuffer[sample] = dry * inRight + wet * inRightFiltered;
		}
	}
}

//==============================================================================
bool BiquadFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BiquadFilterAudioProcessor::createEditor()
{
    return new BiquadFilterAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void BiquadFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BiquadFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout BiquadFilterAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>( FREQUENCY_MIN, FREQUENCY_MAX,  1.0f, 0.3f), 500.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(        -18.0f,         18.0f, 0.01f, 1.0f),   0.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(          0.1f,          72.0, 0.01f, 1.0f),   0.7f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(          0.0f,         100.0,  1.0f, 1.0f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(        -36.0f,         36.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("LP", "LP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("HP", "HP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("AP", "AP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("BP", "BP", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("N", "N", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("P", "P", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("LS", "LS", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("HS", "HS", false));

	layout.add(std::make_unique<juce::AudioParameterBool>("DF1", "DF1", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF2", "DF2", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF1T", "DF1T", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("DF2T", "DF2T", true));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BiquadFilterAudioProcessor();
}
