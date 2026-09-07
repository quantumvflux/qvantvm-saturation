/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <memory>
#include "PluginEditor.h"
#include "BinaryData.h"
#include "PluginProcessor.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_processors_headless/juce_audio_processors_headless.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

//==============================================================================
NekogainAudioProcessorEditor::NekogainAudioProcessorEditor (NekogainAudioProcessor& p)
    :
    AudioProcessorEditor (&p),
    audioProcessor(p),
    apvts (p.apvts)
{
    // gain knob
    gainKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainKnob.setRotaryParameters(-juce::MathConstants<float>::pi * 0.75f,
                                 juce::MathConstants<float>::pi * 0.75f,
                                 true);
    gainKnob.setMouseDragSensitivity (100);
    gainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    gainLabel.setText("DRIVE", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);

    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts,
        "gain",
        gainKnob
    );
    addAndMakeVisible(gainKnob);

    // mix knob
    mixKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixKnob.setRotaryParameters(-juce::MathConstants<float>::pi * 0.75f,
                                juce::MathConstants<float>::pi * 0.75f,
                                true);
    mixKnob.setMouseDragSensitivity (100);
    mixKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts,
        "mix",
        mixKnob
    );
    addAndMakeVisible(mixKnob);

    // output knob
    outputKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outputKnob.setRotaryParameters(-juce::MathConstants<float>::pi * 0.75f,
                                   juce::MathConstants<float>::pi * 0.75f,
                                   true);
    outputKnob.setMouseDragSensitivity (100);
    outputKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    outputLabel.setText("OUTPUT", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);

    outputAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts,
        "output",
        outputKnob
    );
    addAndMakeVisible(outputKnob);

    // bypass button
    bypassButton.setButtonText("BYPASS");
    bypassButton.setClickingTogglesState(true);
    bypassButton.addListener(this);

    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts,
        "bypass",
        bypassButton
    );

    addAndMakeVisible(bypassButton);

    setSize (377, 257);
    setLookAndFeel(&lookAndFeel);

    backgroundImage = juce::ImageCache::getFromMemory( BinaryData::background_png, BinaryData::background_pngSize);
}

void NekogainAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &bypassButton)
    {
        repaint();
    }
}

NekogainAudioProcessorEditor::~NekogainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void NekogainAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
        auto bypassParam = apvts.getRawParameterValue("bypass");
        bool isBypassed = bypassParam->load() > 0.5f;

        g.setColour(isBypassed ? juce::Colour (0xFF4A4845) : juce::Colour (0xFFFFB84D));
        g.fillEllipse(ledArea.toFloat());
    }
    else
    {
        g.fillAll(juce::Colours::black);
    }

}

void NekogainAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    const int knobH = 60;
    const int knobW = 60;
    const int gap = 40;
    const int totalKnobs = 3;

    int totalWidth = totalKnobs * knobW + (totalKnobs - 1) * gap;
    int startX = area.getCentreX() - totalWidth / 2;
    int knobY = 50;

    gainKnob.setBounds(startX, knobY, knobW, knobH);
    gainLabel.setBounds(startX, knobY + knobH + 10, knobW, 20);

    mixKnob.setBounds(startX + (knobW + gap), knobY, knobW, knobH);
    mixLabel.setBounds(startX + (knobW + gap), knobY + knobH + 10, knobW, 20);

    outputKnob.setBounds(startX + 2 * (knobW + gap), knobY, knobW, knobH);
    outputLabel.setBounds(startX + 2 * (knobW + gap), knobY + knobH + 10, knobW, 20);

    const int btnW = 70;
    const int btnH = 25;
    const int ledSize = 15;
    const int ledGap = 10;
    const int btnY = area.getBottom() - 50;

    int totalW = btnW + ledGap + ledSize;
    int bypassStartX = area.getCentreX() - totalW / 2;

    bypassButton.setBounds(bypassStartX, btnY, btnW, btnH);
    ledArea = { bypassStartX + btnW + ledGap, btnY + (btnH - ledSize) / 2, ledSize, ledSize};
}
