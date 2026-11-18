#include "PPMDsp.h"

void PpmDetector::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;

    // Valeurs par défaut v0.2 : 
    // PPM (True Peak)  : 0ms attaque, 1.5s release pour -10dB, 500ms hold
    // QPPM (Modulomètre): 5ms attaque, 0.7s release pour -10dB, 0ms hold
    updateParameters(0.0f, 1.5f, 500.0f, 5.0f, 0.7f, 0.0f);

    float floorGain = juce::Decibels::decibelsToGain(-100.0f);

    envelopePpmL = floorGain;  envelopePpmR = floorGain;
    envelopeQppmL = floorGain; envelopeQppmR = floorGain;

    holdCounterPpmL = 0;  holdCounterPpmR = 0;
    holdCounterQppmL = 0; holdCounterQppmR = 0;
}

void PpmDetector::updateParameters(float attackPpmMs, float releasePpmSec, float holdPpmMs,
    float attackQppmMs, float releaseQppmSec, float holdQppmMs)
{
    // --- 1. CONFIGURATION DU PPM (True Peak) ---
    coeffAttackPpm = (attackPpmMs <= 0.0f) ? 0.0f : std::exp(-1.0f / (currentSampleRate * (attackPpmMs / 1000.0f)));

    // 🟢 CORRECTION : Formule de décroissance exponentielle identique à ton ancien plug
    coeffReleasePpm = 1.0f - std::exp(-1.5f / (currentSampleRate * juce::jmax(0.01f, releasePpmSec)));

    holdSamplesPpm = juce::roundToInt((holdPpmMs / 1000.0f) * currentSampleRate);

    // --- 2. CONFIGURATION DU QPPM (Modulomètre) ---
    coeffAttackQppm = (attackQppmMs <= 0.0f) ? 0.0f : std::exp(-1.60944f / (currentSampleRate * (attackQppmMs / 1000.0f)));

    // 🟢 CORRECTION : Même chose pour le QPPM
    coeffReleaseQppm = 1.0f - std::exp(-1.5f / (currentSampleRate * juce::jmax(0.01f, releaseQppmSec)));

    holdSamplesQppm = juce::roundToInt((holdQppmMs / 1000.0f) * currentSampleRate);
}

void PpmDetector::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples <= 0 || numChannels <= 0) return;

    auto* channelDataL = buffer.getReadPointer(0);
    auto* channelDataR = (numChannels > 1) ? buffer.getReadPointer(1) : channelDataL;

    for (int i = 0; i < numSamples; ++i)
    {
        float absL = std::abs(channelDataL[i]);
        float absR = std::abs(channelDataR[i]);

        processSample(absL, envelopePpmL, envelopeQppmL, holdCounterPpmL, holdCounterQppmL);
        processSample(absR, envelopePpmR, envelopeQppmR, holdCounterPpmR, holdCounterQppmR);
    }
}

void PpmDetector::processSample(float inputSample, float& envPpm, float& envQppm,
    int& holdCountPpm, int& holdCountQppm)
{
    float cleanInput = juce::jmax(0.00001f, inputSample); // Amplitude linéaire (0.0 à 1.0)

    // --- 1. BALISTIQUE DU PPM (True Peak type ancien) ---
    if (cleanInput > envPpm)
    {
        // Attaque immédiate ou filtrée
        envPpm = (coeffAttackPpm == 0.0f) ? cleanInput : (1.0f - coeffAttackPpm) * cleanInput + coeffAttackPpm * envPpm;
        holdCountPpm = holdSamplesPpm; // On charge le hold
    }
    else
    {
        if (holdCountPpm > 0)
        {
            holdCountPpm--; // On maintient le niveau haut
        }
        else
        {
            // 🟢 ICI : Décroissance EXPONENTIELLE en amplitude (comme ton ancien code !)
            // Au lieu de faire du pas à pas en dB, on applique le coefficient de lissage
            envPpm += coeffReleasePpm * (cleanInput - envPpm);
        }
    }

    // --- 2. BALISTIQUE DU QPPM ---
    if (cleanInput > envQppm)
    {
        envQppm = (1.0f - coeffAttackQppm) * cleanInput + coeffAttackQppm * envQppm;
        holdCountQppm = holdSamplesQppm;
    }
    else
    {
        if (holdCountQppm > 0)
        {
            holdCountQppm--;
        }
        else
        {
            // 🟢 Idem pour le QPPM
            envQppm += coeffReleaseQppm * (cleanInput - envQppm);
        }
    }
}