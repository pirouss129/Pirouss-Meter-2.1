#pragma once
#include <JuceHeader.h>
#include <array>

namespace RM3Config
{
    // LUT des références K-System correspondant exactement aux index APVTS (0, 1, 2, 3)
    static inline const std::array<float, 4> kSystemReferences = {
        -20.0f, // Index 0 : K-20
        -18.0f, // Index 1 : K-18 (Défaut)
        -14.0f, // Index 2 : K-14
        -12.0f  // Index 3 : K-12
    };
}

class RM3Bargraph : public juce::Component
{
public:
    enum class ScalePosition
    {
        Left,
        Right,
        None
    };

    RM3Bargraph();
    ~RM3Bargraph() override = default;

    void setLevels(float rmsShort, float rmsMom);
    void setScalePosition(ScalePosition pos) { scalePos = pos; repaint(); }
    void setChannelName(const juce::String& name) { channelName = name; repaint(); }

    // 🟢 MÉTHODE LUT : Mise à jour par Index (0, 1, 2, 3)
    void setKSystemModeByIndex(int index)
    {
        auto safeIdx = static_cast<size_t>(juce::jlimit(0, static_cast<int>(RM3Config::kSystemReferences.size()) - 1, index));
        setKSystemMode(RM3Config::kSystemReferences[safeIdx]);
    }

    // 🟢 Changement direct de référence K-System (-20, -18, -14, -12 dBFS)
    void setKSystemMode(float referenceDb)
    {
        if (kRefDb != referenceDb)
        {
            kRefDb = referenceDb;
            repaint();
        }
    }

    // 🟢 Switch entre mode Relatif (K) et Absolu (dBFS)
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
    void drawSquareLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float valBarDb, float valLedDb);
    juce::Colour getSegmentColour(float db) const;

    float rmsS = -60.0f;
    float rmsM = -60.0f;

    float minDb = -50.0f;
    float maxDb = 0.0f;

    // 🟢 RÉFÉRENCE DU 0 dB K-SYSTEM PAR DÉFAUT (K-18)
    float kRefDb = -18.0f;
    bool isAbsolute = false; // false = K-System, true = dBFS

    int numSegments = 40;
    ScalePosition scalePos = ScalePosition::Left;
    juce::String channelName = "RMS";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RM3Bargraph)
};