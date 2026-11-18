#pragma once
#include <JuceHeader.h>

class PiroussHeader : public juce::Component
{
public:
    // Constructeur : reçoit le nom du plugin et la couleur du thème
    PiroussHeader(const juce::String& pluginName, const juce::String& pluginVersion, const juce::Colour themeColor);

    void paint(juce::Graphics& g) override;

private:
    juce::String name;
    juce::String version;
    juce::Colour color;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PiroussHeader)
};