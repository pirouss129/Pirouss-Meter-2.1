#pragma once

#include <JuceHeader.h>

extern "C"
{
#include "ebur128.h"
}

class Ebu128LoudnessMeter
{
public:
    Ebu128LoudnessMeter() = default;

    ~Ebu128LoudnessMeter()
    {
        destroyState();
    }

    void prepareToPlay(double sampleRate, int numChannels, int /*samplesPerBlock*/, int /*updateRateHz*/)
    {
        destroyState();

        currentChannels = static_cast<unsigned int>(numChannels);

        // Configuration des modes de calcul EBU R128 :
        // EBUR128_MODE_M   -> Momentary (400 ms)
        // EBUR128_MODE_S   -> Short-Term (3 s)
        // EBUR128_MODE_I   -> Integrated (Loudness intégrée globale)
        // EBUR128_MODE_LRA -> Loudness Range (LRA)
        int mode = EBUR128_MODE_M | EBUR128_MODE_S | EBUR128_MODE_I | EBUR128_MODE_LRA;

        st = ebur128_init(currentChannels, static_cast<unsigned long>(sampleRate), mode);

        // Configuration du routage des canaux par défaut pour ebur128
        if (st != nullptr)
        {
            if (currentChannels == 1)
            {
                ebur128_set_channel(st, 0, EBUR128_CENTER);
            }
            else if (currentChannels == 2)
            {
                ebur128_set_channel(st, 0, EBUR128_LEFT);
                ebur128_set_channel(st, 1, EBUR128_RIGHT);
            }
        }
    }

    void reset()
    {
        if (st != nullptr)
        {
            // On conserve la config actuelle pour réinitialiser le state
            unsigned int channels = st->channels;
            unsigned long samplerate = st->samplerate;
            int mode = st->mode;

            ebur128_destroy(&st);
            st = ebur128_init(channels, samplerate, mode);

            if (st != nullptr)
            {
                if (channels == 1)
                {
                    ebur128_set_channel(st, 0, EBUR128_CENTER);
                }
                else if (channels == 2)
                {
                    ebur128_set_channel(st, 0, EBUR128_LEFT);
                    ebur128_set_channel(st, 1, EBUR128_RIGHT);
                }
            }
        }
    }

    void processBlock(const juce::AudioBuffer<float>& buffer)
    {
        if (st == nullptr) return;

        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        if (numSamples == 0 || numChannels == 0) return;

        // libebur128 attend un buffer audio entrelacé (interleaved : L, R, L, R...)
        interleavedBuffer.resize(static_cast<size_t>(numSamples * numChannels));

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* channelData = buffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                interleavedBuffer[static_cast<size_t>(i * numChannels + ch)] = channelData[i];
            }
        }

        // On envoie les frames entrelacées à libebur128
        ebur128_add_frames_float(st, interleavedBuffer.data(), static_cast<size_t>(numSamples));
    }

    float getMomentaryLoudness() const
    {
        if (st == nullptr) return -100.0f;
        double val = -100.0;
        if (ebur128_loudness_momentary(st, &val) == EBUR128_SUCCESS)
            return static_cast<float>(val);
        return -100.0f;
    }

    float getShortTermLoudness() const
    {
        if (st == nullptr) return -100.0f;
        double val = -100.0;
        if (ebur128_loudness_shortterm(st, &val) == EBUR128_SUCCESS)
            return static_cast<float>(val);
        return -100.0f;
    }

    float getIntegratedLoudness() const
    {
        if (st == nullptr) return -100.0f;
        double val = -100.0;
        if (ebur128_loudness_global(st, &val) == EBUR128_SUCCESS)
            return static_cast<float>(val);
        return -100.0f;
    }

    float getLoudnessRange() const
    {
        if (st == nullptr) return 0.0f;
        double val = 0.0;
        if (ebur128_loudness_range(st, &val) == EBUR128_SUCCESS)
            return static_cast<float>(val);
        return 0.0f;
    }

private:
    void destroyState()
    {
        if (st != nullptr)
        {
            ebur128_destroy(&st);
            st = nullptr;
        }
    }

    ebur128_state* st = nullptr;
    unsigned int currentChannels = 2;
    std::vector<float> interleavedBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Ebu128LoudnessMeter)
};