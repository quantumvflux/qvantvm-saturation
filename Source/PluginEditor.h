/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include "PluginLookAndFeel.h"

//==============================================================================
/**
*/
class NekogainAudioProcessor;
class NekogainAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    NekogainAudioProcessorEditor (NekogainAudioProcessor&);
    ~NekogainAudioProcessorEditor() override;
    void buttonClicked(juce::Button*) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NekogainAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;

    juce::Slider gainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;
    juce::Label gainLabel;

    juce::Slider mixKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;
    juce::Label mixLabel;

    juce::Slider outputKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttach;
    juce::Label outputLabel;

    juce::TextButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    juce::Rectangle<int> ledArea;

    PluginLookAndFeel lookAndFeel;

    juce::Image backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NekogainAudioProcessorEditor)
};
