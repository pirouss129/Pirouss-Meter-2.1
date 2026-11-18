#pragma once
#include <JuceHeader.h>

class TPDsp
{
public:
    TPDsp();
    ~TPDsp() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    // Traite le buffer audio et renvoie le niveau True Peak max (en dBFS)
    float processBlock(const juce::AudioBuffer<float>& buffer);

    float getTruePeak() const { return maxTruePeakDb; }

    void resetMax() { maxTruePeakDb = -70.0f; }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    float maxTruePeakDb = -70.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TPDsp)
};