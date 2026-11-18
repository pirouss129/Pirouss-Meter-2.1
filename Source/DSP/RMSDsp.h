// Dans RMSDsp.h
#pragma once
#include <JuceHeader.h>

class RMSDsp
{
public:
    RMSDsp() = default;
    ~RMSDsp() = default;

    void prepare(double sampleRate);
    void processBlock(const juce::AudioBuffer<float>& buffer);
    void updateParameters(float windowM_ms, float windowS_ms);
    void setKWeightingEnabled(bool enabled) { kWeightingEnabled = enabled; }

    // Getters propres pour l'UI
    float getRmsML() const { return rmsM_L_dB; }
    float getRmsMR() const { return rmsM_R_dB; }
    float getRmsSL() const { return rmsS_L_dB; }
    float getRmsSR() const { return rmsS_R_dB; }

    // 🟢 NOUVEAUX GETTERS MONO (Sommation L+R pure, sans AES17)
    float getRmsMonoM() const { return rmsM_Mono_dB; } // Mono Momentary
    float getRmsMonoS() const { return rmsS_Mono_dB; } // Mono Short-Term

    
private:
    void updateFilterCoefficients();

    double currentSampleRate = 44100.0;
    bool kWeightingEnabled = false;

    
    // 🟢 STRUCTURES FILTRES ISSUES DU PIROUSS K (Module juce::dsp)
    juce::dsp::IIR::Filter<float> headFilterL, headFilterR;
    juce::dsp::IIR::Filter<float> hipassFilterL, hipassFilterR;

    float coeffM = 0.0f;
    float coeffS = 0.0f;

    // Mémoires d'enveloppes Stéréo (Puissance au carré)
    float rmsM_L = 0.0f, rmsM_R = 0.0f;
    float rmsS_L = 0.0f, rmsS_R = 0.0f;

    // 🟢 Mémoires d'enveloppes Mono (Sommation de puissance au carré)
    float rmsM_Mono = 0.0f;
    float rmsS_Mono = 0.0f;

    // Valeurs finales en dB (Stéréo avec AES17)
    float rmsM_L_dB = -100.0f, rmsM_R_dB = -100.0f;
    float rmsS_L_dB = -100.0f, rmsS_R_dB = -100.0f;

    // 🟢 Valeurs finales en dB (Mono pure)
    float rmsM_Mono_dB = -100.0f;
    float rmsS_Mono_dB = -100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RMSDsp)
};