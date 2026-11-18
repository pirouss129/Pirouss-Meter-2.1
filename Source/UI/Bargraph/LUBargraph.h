#pragma once
#include <JuceHeader.h>
#include <array>

namespace LUConfig
{
    // LUT des cibles LUFS correspondant aux index APVTS (0, 1, 2)
    static inline const std::array<float, 3> luTargetReferences = {
        -23.0f, // Index 0 : EBU R128 (-23 LUFS)
        -19.0f, // Index 1 : Target -19 LUFS
        -15.0f  // Index 2 : Target -15 LUFS
    };
}

class LUBargraph : public juce::Component
{
public:
    LUBargraph();
    ~LUBargraph() override = default;

    void setLevels(float luMomentary, float luShortTerm, float lra, float luIntegrated);

    // 🟢 MÉTHODE LUT : Mise à jour par Index (0, 1, 2)
    void setTargetLUFSByIndex(int index)
    {
        auto safeIdx = static_cast<size_t>(juce::jlimit(0, static_cast<int>(LUConfig::luTargetReferences.size()) - 1, index));
        setTargetLUFS(LUConfig::luTargetReferences[safeIdx]);
    }

    void setTargetLUFS(float target)
    {
        if (targetLUFS != target)
        {
            targetLUFS = target;
            repaint();
        }
    }

    void setDisplayMode(bool absoluteMode)
    {
        if (isAbsolute != absoluteMode)
        {
            isAbsolute = absoluteMode;
            repaint();
        }
    }

    // Méthode pour changer la couleur de bordure
    juce::Colour borderColour = juce::Colours::ghostwhite.withAlpha(0.5f);
    void setBorderColour(const juce::Colour& newColour)
    {
        borderColour = newColour;
        repaint();
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // 🟢 Méthode de dessin d'un bargraphe individuel
    void drawSingleLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float value, bool isMomentary);
    void drawLraLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float lraVal, float integratedLu);
    juce::Colour getSegmentColour(float segDb, bool isMomentary) const;
    juce::Colour getLraSegmentColour(float segLu) const;

    // 🎨 Fonctions de couleur
    juce::Colour getLusColour(float valLUFS) const;
    juce::Colour getLumColour(float valLUFS) const;

    float luS = -70.0f;
    float luM = -70.0f;
    float lraValue = -1.0f;
    float luI = -70.0f;

    // 🟢 Variables d'affichage et de mode
    float targetLUFS = -23.0f;
    bool isAbsolute = false;

    // Nb de leds par bargraph
    int numSegments = 50;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LUBargraph)
};