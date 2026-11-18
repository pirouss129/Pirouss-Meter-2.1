#pragma once
#include <JuceHeader.h>

class DigitalDisplay : public juce::Component
{
public:
    DigitalDisplay(const juce::String& labelText, juce::Colour valueColour);
    ~DigitalDisplay() override = default;

    void setValue(float newValue);
    void setLabel(const juce::String& newLabel);

    // Méthode pour changer la couleur de bordure
    juce::Colour borderColour = juce::Colours::ghostwhite.withAlpha(0.5f); // Couleur par défaut de la bordure
    void setBorderColour(const juce::Colour& newColour)
    {
        borderColour = newColour;
        repaint();
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::String label;
    float value = -70.0f;
    juce::Colour displayColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DigitalDisplay)
};