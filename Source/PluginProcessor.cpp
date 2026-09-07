/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"
#include "juce_core/juce_core.h"
#include "juce_data_structures/juce_data_structures.h"
#include "juce_dsp/juce_dsp.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <cmath>
#include <memory>

//==============================================================================
NekogainAudioProcessor::NekogainAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, apvts (*this, nullptr, "Parameters", createParameterLayout()), oversampler(2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple)
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
NekogainAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID {"gain", 1},
        "Gain",
        juce::NormalisableRange<float> (-12.0f, 24.0f, 0.1f, 0.5f),
        0.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"mix", 1},
        "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f, 1.0f),
        0.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"output", 1},
        "Output",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f, 1.0f),
        0.0f
    ));

    layout.add (std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID {"bypass", 1},
        "Bypass",
        false
    ));

    return layout;
}

NekogainAudioProcessor::~NekogainAudioProcessor()
{
}

//==============================================================================
const juce::String NekogainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NekogainAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NekogainAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NekogainAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NekogainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NekogainAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NekogainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NekogainAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NekogainAudioProcessor::getProgramName (int index)
{
    return {};
}

void NekogainAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NekogainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    oversampler.reset();
    oversampler.initProcessing ((size_t) samplesPerBlock);
    setLatencySamples((int) oversampler.getLatencyInSamples());
}

void NekogainAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NekogainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NekogainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto* drive = apvts.getRawParameterValue ("gain");
    float gain = juce::Decibels::decibelsToGain(drive->load());

    auto* mixer = apvts.getRawParameterValue("mix");
    float mix = mixer->load();

    auto* output = apvts.getRawParameterValue("output");
    float outputGain = juce::Decibels::decibelsToGain(output->load());

    auto* bypassParam = apvts.getRawParameterValue("bypass");
    bool bypassed = bypassParam->load();

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler.processSamplesUp(block);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer (channel);
        saturationDSP.setDrive(gain);

        for(int sample = 0; sample < (int) oversampledBlock.getNumSamples(); ++sample)
        {
            float dry = channelData[sample];
            float wet = saturationDSP.processSample(dry);
            float mixed = dry * (1.0f - mix) + wet * mix;

            channelData[sample] =  bypassed ? dry : mixed;
        }
    }
    oversampler.processSamplesDown (block);
    buffer.applyGain(outputGain);
}

//==============================================================================
bool NekogainAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NekogainAudioProcessor::createEditor()
{
    return new NekogainAudioProcessorEditor (*this);
}

//==============================================================================
void NekogainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NekogainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NekogainAudioProcessor();
}
