#pragma once
#include <JuceHeader.h>

class L_Switch : public juce::Component
{
public:
    juce::TextButton myButton;

    L_Switch(const juce::String& offText = "OFF",
        const juce::String& onText = "ON",
        const juce::Colour onColor = juce::Colours::chartreuse,
        const juce::Colour offColor = juce::Colours::darkgrey,
        const juce::Colour bgOffColor = juce::Colour(0xff121212),
        const juce::Colour bgOnColor = juce::Colours::black)
    {
        myButton.setClickingTogglesState(true);
        myButton.setToggleState(false, juce::dontSendNotification);
        myButton.setButtonText(offText);

        // Application du LookAndFeel customisé
        myButton.setLookAndFeel(&customLAF);

        // Couleurs de texte natives JUCE
        myButton.setColour(juce::TextButton::textColourOffId, offColor);
        myButton.setColour(juce::TextButton::textColourOnId, onColor);

        // Stockage des couleurs pour notre LAF
        customLAF.setButtonColours(offColor, onColor, bgOffColor, bgOnColor);

        // Gestion dynamique du texte
        myButton.onStateChange = [this, offText, onText]() {
            if (myButton.getToggleState())
                myButton.setButtonText(onText);
            else
                myButton.setButtonText(offText);

            myButton.repaint();
            };

        addAndMakeVisible(myButton);
    }

    ~L_Switch() override
    {
        myButton.setLookAndFeel(nullptr);
    }

    void resized() override
    {
        myButton.setBounds(getLocalBounds());
    }

private:
    class SwitchCustomLAF : public juce::LookAndFeel_V4
    {
    public:
        void setButtonColours(juce::Colour textOff, juce::Colour textOn, juce::Colour bgOff, juce::Colour bgOn)
        {
            offColour = textOff;
            onColour = textOn;
            bgOffColour = bgOff;
            bgOnColour = bgOn;
        }

        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
            bool isOn = button.getToggleState();

            auto currentOutline = isOn ? onColour : offColour;
            auto currentBg = isOn ? bgOnColour : bgOffColour;

            if (shouldDrawButtonAsHighlighted)
                currentBg = currentBg.brighter(0.1f);

            // 1. Fond du bouton
            g.setColour(currentBg);
            g.fillRoundedRectangle(bounds, 3.0f);

            // 2. Contour dynamique
            g.setColour(currentOutline);
            g.drawRoundedRectangle(bounds, 3.0f, 1.5f);
        }

        // Affichage du texte centré (sans la LED)
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto font = getTextButtonFont(button, button.getHeight());
            g.setFont(font);

            auto bounds = button.getLocalBounds().toFloat();
            bool isOn = button.getToggleState();

            // Couleur du texte selon l'état
            auto textColor = isOn ? onColour : offColour;
            g.setColour(textColor);

            // Dessin du texte parfaitement centré
            g.drawFittedText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
        }

    private:
        juce::Colour offColour{ juce::Colours::darkgrey };
        juce::Colour onColour{ juce::Colours::chartreuse };
        juce::Colour bgOffColour{ juce::Colour(0xff222222) };
        juce::Colour bgOnColour{ juce::Colours::black };
    };

    SwitchCustomLAF customLAF;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(L_Switch)
};