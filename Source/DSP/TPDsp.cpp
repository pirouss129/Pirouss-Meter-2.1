#include "TPDsp.h"

TPDsp::TPDsp()
{
    // Oversampling x4 (2^2 = 4) avec filtre RIF à phase linéaire
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        2, // 2 canaux (stéréo)
        2, // Facteur 2^2 = x4
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true
    );
}

void TPDsp::prepare(const juce::dsp::ProcessSpec& spec)
{
    oversampler->initProcessing(spec.maximumBlockSize);
    oversampler->reset();
    maxTruePeakDb = -70.0f;
}

void TPDsp::reset()
{
    if (oversampler)
        oversampler->reset();
    maxTruePeakDb = -70.0f;
}

float TPDsp::processBlock(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return maxTruePeakDb;

    juce::dsp::AudioBlock<const float> inputBlock(buffer);
    auto oversampledBlock = oversampler->processSamplesUp(inputBlock);

    float blockPeakLinear = 0.0f;

    for (size_t ch = 0; ch < oversampledBlock.getNumChannels(); ++ch)
    {
        auto* samples = oversampledBlock.getChannelPointer(ch);
        for (size_t i = 0; i < oversampledBlock.getNumSamples(); ++i)
        {
            float absSample = std::abs(samples[i]);
            if (absSample > blockPeakLinear)
                blockPeakLinear = absSample;
        }
    }

    // Convertir la crête du block courant
    float currentBlockDb = -70.0f;
    if (blockPeakLinear > 0.00003162278f) // > -90 dBFS
        currentBlockDb = juce::Decibels::gainToDecibels(blockPeakLinear);

    // 🟢 Peak Hold : On ne garde que le MAX absolu
    if (currentBlockDb > maxTruePeakDb)
        maxTruePeakDb = currentBlockDb;

    return maxTruePeakDb;
}