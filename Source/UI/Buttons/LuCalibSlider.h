#pragma once
#include <JuceHeader.h>

class L_CalibSlider : public juce::Component
{
public:
    // --- LOOK AND FEEL LOCALISÉ (Fader carré plus fin et rail visible) ---
    class VintageSliderLAF : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPos, float minSliderPos, float maxSliderPos,
            const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            if (style == juce::Slider::LinearHorizontal)
            {
                auto trackColor = slider.findColour(juce::Slider::trackColourId);
                auto bgColor = slider.findColour(juce::Slider::backgroundColourId);
                auto thumbColor = slider.findColour(juce::Slider::thumbColourId);

                // 1. Dessiner le fond du rail (la rigole creusée)
                auto trackHeight = 4.0f;
                auto trackY = (float)y + ((float)height - trackHeight) * 0.5f;
                juce::Rectangle<float> trackBounds((float)x + 4.0f, trackY, (float)width - 8.0f, trackHeight);

                g.setColour(bgColor);
                g.fillRoundedRectangle(trackBounds, 2.0f);

                g.setColour(juce::Colours::black.withAlpha(0.8f));
                g.drawRoundedRectangle(trackBounds, 2.0f, 1.0f);

                // 2. Dessiner la partie active du rail (remplie jusqu'au curseur)
                juce::Rectangle<float> filledTrackBounds((float)x + 4.0f, trackY, sliderPos - ((float)x + 4.0f), trackHeight);
                g.setColour(trackColor.withAlpha(0.6f));
                g.fillRoundedRectangle(filledTrackBounds, 2.0f);

                // 3. Dessiner le curseur carré (plus petit et élégant)
                float thumbWidth = 10.0f;  // Plus étroit
                float thumbHeight = (float)height - 6.0f; // Moins haut pour laisser respirer le rail
                float thumbX = sliderPos - (thumbWidth / 2.0f);
                float thumbY = (float)y + ((float)height - thumbHeight) * 0.5f; // Centré verticalement

                juce::Rectangle<float> thumbRect(thumbX, thumbY, thumbWidth, thumbHeight);

                // Corps du fader opaque
                g.setColour(thumbColor);
                g.fillRect(thumbRect);

                // Contour sombre pour le relief
                g.setColour(thumbColor.darker(0.8f));
                g.drawRect(thumbRect, 1.0f);

                // Biseau lumineux sur le haut et la gauche
                g.setColour(thumbColor.brighter(0.6f));
                g.drawHorizontalLine(static_cast<int>(thumbRect.getY()), thumbRect.getX(), thumbRect.getRight());
                g.drawVerticalLine(static_cast<int>(thumbRect.getX()), thumbRect.getY(), thumbRect.getBottom());

                // Petite rainure centrale noire
                g.setColour(juce::Colours::black.withAlpha(0.7f));
                g.fillRect(thumbRect.getCentreX() - 0.5f, thumbRect.getY() + 2.0f, 1.0f, thumbRect.getHeight() - 4.0f);
            }
            else
            {
                LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            }
        }
    };

    VintageSliderLAF customLAF;
    juce::Slider slider;

    L_CalibSlider(juce::Colour color = juce::Colours::red)
    {
        slider.setLookAndFeel(&customLAF);

        // --- CONFIG SLIDER ---
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setColour(juce::Slider::thumbColourId, color);
        slider.setColour(juce::Slider::trackColourId, color);
        //slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff080808)); // Fond creusé
        slider.setColour(juce::Slider::backgroundColourId, color.darker(0.3f)); // Fond creusé

        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 16);
        slider.setColour(juce::Slider::textBoxTextColourId, color);
        slider.setColour(juce::Slider::textBoxOutlineColourId, color);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0d0d0d));

        slider.setDoubleClickReturnValue(true, 0.0);

        addAndMakeVisible(slider);
    }

    ~L_CalibSlider() override
    {
        slider.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        slider.setBounds(bounds);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(L_CalibSlider)
};