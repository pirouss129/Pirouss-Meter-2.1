#pragma once
#include <JuceHeader.h>

class RTW40Bargraph : public juce::Component
{
public:
    RTW40Bargraph();
    ~RTW40Bargraph() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Méthode pour changer la couleur de bordure
    juce::Colour borderColour = juce::Colours::ghostwhite.withAlpha(0.5f); // Couleur par défaut de la bordure
    void setBorderColour(const juce::Colour& newColour)
    {
        borderColour = newColour;
        repaint();
    }

    void setLevels(float qppmL, float qppmR, float ppmHoldL = -100.0f, float ppmHoldR = -100.0f); // méthode à appeler pour dessiner le niveau dans le bargraph

private:
    float levelL = -100.0f, levelR = -100.0f;
    float peakL = -100.0f, peakR = -100.0f;

    static constexpr int numSegments = 50; //40
    static constexpr float minDb = -50.0f;
    static constexpr float maxDb = 0.0f;

    juce::Colour getSegmentColour(float db) const;
    void drawSingleBar(juce::Graphics& g, juce::Rectangle<float> bounds, float valDb, float holdDb);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTW40Bargraph)
};