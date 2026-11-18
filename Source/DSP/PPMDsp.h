#pragma once
#include <JuceHeader.h>

class PpmDetector
{
public:
    PpmDetector() = default;
    ~PpmDetector() = default;

    // Prépare le DSP (calcul des coefficients initiaux)
    void prepare(double sampleRate);

    // Met à jour les temps si tu modifies un slider ou un paramètre
    void updateParameters(float attackPpmMs, float releasePpmSec, float holdPpmMs,
        float attackQppmMs, float releaseQppmSec, float holdQppmMs);

    // Traite le buffer stéréo complet
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Getters pour récupérer les valeurs dans l'UI (en dBFS)
    float getPpmLeft() const { return juce::Decibels::gainToDecibels(envelopePpmL); }
    float getPpmRight() const { return juce::Decibels::gainToDecibels(envelopePpmR); }

    float getQppmLeft() const { return juce::Decibels::gainToDecibels(envelopeQppmL); }
    float getQppmRight() const { return juce::Decibels::gainToDecibels(envelopeQppmR); }

    
private:
    

    double currentSampleRate = 44100.0;

    // --- COEFFICIENTS DE BALISTIQUE ---
    float coeffAttackPpm = 0.0f;
    float coeffReleasePpm = 0.0f; // Exprimé en pas linéaire (dB par sample)
    int   holdSamplesPpm = 0;

    float coeffAttackQppm = 0.0f;
    float coeffReleaseQppm = 0.0f; // Exprimé en pas linéaire (dB par sample)
    int   holdSamplesQppm = 0;

    // --- MÉMOIRES DES ENVELOPPES (en Linéaire / Gain) ---
    float envelopePpmL = 0.0f;
    float envelopePpmR = 0.0f;

    float envelopeQppmL = 0.0f;
    float envelopeQppmR = 0.0f;

    // --- COMPTEURS DE HOLD (en nombres de samples restants) ---
    int holdCounterPpmL = 0;
    int holdCounterPpmR = 0;
    int holdCounterQppmL = 0;
    int holdCounterQppmR = 0;

    // Petite fonction utilitaire interne pour traiter un canal individuellement
    void processSample(float inputSample, float& envPpm, float& envQppm,
        int& holdCountPpm, int& holdCountQppm);
};