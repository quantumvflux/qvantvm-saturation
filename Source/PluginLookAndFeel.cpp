#include "PluginLookAndFeel.h"
#include "juce_core/juce_core.h"

namespace {
    constexpr float knobStartAngle = -juce::MathConstants<float>::pi * 0.75f; // -135 grados
    constexpr float knobEndAngle = juce::MathConstants<float>::pi * 0.75f; // +135 grados
}

void PluginLookAndFeel::drawRotarySlider(
    juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float radius = juce::jmin(width, height) * 0.5f - 2.0f;

    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, knobStartAngle, knobEndAngle, true);

    g.setColour(juce::Colour(0xFF232634));
    g.strokePath(arcPath, juce::PathStrokeType(2.0f));

    g.setColour(juce::Colour(0xFFDFDFDF));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    const float angle = knobStartAngle + sliderPosProportional * (knobEndAngle - knobStartAngle);
    const float indicatorLength = radius * 0.75f;
    const float endX = centreX + indicatorLength * std::sin(angle);
    const float endY = centreY - indicatorLength * std::cos(angle);

    g.setColour (juce::Colours::black);
    g.drawLine (centreX, centreY, endX, endY, 3.0f);
}
