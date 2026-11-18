#include "RMSDsp.h"
#include <cmath>

void RMSDsp::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;

    updateParameters(150.0f, 600.0f);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 1;

    headFilterL.prepare(spec);   headFilterR.prepare(spec);
    hipassFilterL.prepare(spec); hipassFilterR.prepare(spec);

    updateFilterCoefficients();

    // Reset mémoires Stéréo
    rmsM_L = 0.0f; rmsM_R = 0.0f;
    rmsS_L = 0.0f; rmsS_R = 0.0f;

    // 🟢 Reset mémoires Mono
    rmsM_Mono = 0.0f;
    rmsS_Mono = 0.0f;
}

void RMSDsp::updateFilterCoefficients()
{
    // Coefficients exacts de la norme ITU-R BS.1770 / EBU R128
    double f0_shelf = 1681.974450584374;
    double gain_shelf = 3.999847718151522;
    double Q_shelf = 0.7071752369554193;

    auto shelfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, f0_shelf, Q_shelf, juce::Decibels::decibelsToGain(gain_shelf));
    *headFilterL.coefficients = *shelfCoeffs;
    *headFilterR.coefficients = *shelfCoeffs;

    double f0_hpf = 38.13547087613982;
    double Q_hpf = 0.5003270373238773;

    auto hpfCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, f0_hpf, Q_hpf);
    *hipassFilterL.coefficients = *hpfCoeffs;
    *hipassFilterR.coefficients = *hpfCoeffs;
}

void RMSDsp::updateParameters(float windowM_ms, float windowS_ms)
{
    // Un seul coefficient symétrique par enveloppe (Attaque = Release)
    coeffM = 1.0f - std::exp(-1.0f / (currentSampleRate * (windowM_ms / 1000.0f)));
    coeffS = 1.0f - std::exp(-1.0f / (currentSampleRate * (windowS_ms / 1000.0f)));
}

void RMSDsp::processBlock(const juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    if (numSamples <= 0) return;

    const float* srcL = buffer.getReadPointer(0);
    const float* srcR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : srcL;

    for (int i = 0; i < numSamples; ++i)
    {
        float sampleL = srcL[i];
        float sampleR = srcR[i];

        if (kWeightingEnabled)
        {
            sampleL = headFilterL.processSample(sampleL);
            sampleL = hipassFilterL.processSample(sampleL);

            sampleR = headFilterR.processSample(sampleR);
            sampleR = hipassFilterR.processSample(sampleR);

            // Debug : si K-w décalage gauche droite
            //sampleL = sampleL * 2.0f;
            //sampleR = sampleR * 0.5f;
        }

        // 1. Puissance instantanée par canal
        float sqrL = sampleL * sampleL;
        float sqrR = sampleR * sampleR;

        // Intégration Stéréo
        rmsM_L += coeffM * (sqrL - rmsM_L);
        rmsM_R += coeffM * (sqrR - rmsM_R);
        rmsS_L += coeffS * (sqrL - rmsS_L);
        rmsS_R += coeffS * (sqrR - rmsS_R);

        // 🟢 2. Sommation Mono de l'énergie (Moyenne des puissances)
        float sqrMono = (sqrL + sqrR) * 0.5f;

        // Intégration Mono indépendante
        rmsM_Mono += coeffM * (sqrMono - rmsM_Mono); // Fenêtre Momentary
        rmsS_Mono += coeffS * (sqrMono - rmsS_Mono); // Fenêtre Short-Term
    }

    // 3. Extraction Stéréo avec alignement AES17 (+3.01 dB)
    auto powerToDbAES17 = [](float power) -> float
        {
            if (power <= 0.0000000001f) return -100.0f;
            float rmsAmplitude = std::sqrt(power);
            float rmsDb = (20.0f * std::log10(rmsAmplitude)) + 3.01f;
            return std::min(0.0f, rmsDb);
        };

    rmsM_L_dB = powerToDbAES17(rmsM_L);
    rmsM_R_dB = powerToDbAES17(rmsM_R);
    rmsS_L_dB = powerToDbAES17(rmsS_L);
    rmsS_R_dB = powerToDbAES17(rmsS_R);

    // 🟢 4. EXTRACTION MONO MATHÉMATIQUE PURE (Sans décalage AES17)
    auto powerToDbMathPure = [](float power) -> float
        {
            if (power <= 0.0000000001f) return -100.0f;
            float rmsAmplitude = std::sqrt(power);
            float rmsDb = 20.0f * std::log10(rmsAmplitude); // Pas de +3.01 dB ici !
            return std::min(0.0f, rmsDb);
        };

    rmsM_Mono_dB = powerToDbMathPure(rmsM_Mono);
    rmsS_Mono_dB = powerToDbMathPure(rmsS_Mono);
}