#pragma once
#include <JuceHeader.h>

class K_Switch : public juce::Component
{
public:
    juce::TextButton myButton;

    K_Switch(const juce::String& offText = "OFF",
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

    ~K_Switch() override
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

        // On surcharge l'affichage du texte pour dessiner la LED juste à côté
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto font = getTextButtonFont(button, button.getHeight());
            g.setFont(font);

            auto bounds = button.getLocalBounds().toFloat();
            bool isOn = button.getToggleState();

            // Couleur du texte et de la LED selon l'état
            auto textColor = isOn ? onColour : offColour;
            g.setColour(textColor);

            // --- DESSIN DE LA LED ---
            float ledRadius = 3.5f; // Taille de la LED
            float ledX = bounds.getX() + 14.0f; // Position horizontale (décalé du bord gauche)
            float ledY = bounds.getCentreY();   // Centré verticalement

            // Si le bouton est allumé, on met un effet lumineux (halo) autour de la LED
            if (isOn)
            {
                g.setColour(onColour.withAlpha(0.3f));
                g.fillEllipse(ledX - ledRadius * 1.8f, ledY - ledRadius * 1.8f, ledRadius * 3.6f, ledRadius * 3.6f);
            }

            // Corps de la LED
            g.setColour(textColor);
            g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);

            // --- DESSIN DU TEXTE (légèrement décalé vers la droite pour laisser la place à la LED) ---
            auto textBounds = bounds.translated(10.0f, 0.0f); // Petit décalage global pour centrer l'ensemble visuellement
            g.drawFittedText(button.getButtonText(), textBounds.toNearestInt(), juce::Justification::centred, 1);
        }

    private:
        juce::Colour offColour{ juce::Colours::darkgrey };
        juce::Colour onColour{ juce::Colours::chartreuse };
        juce::Colour bgOffColour{ juce::Colour(0xff222222) };
        juce::Colour bgOnColour{ juce::Colours::black };
    };

    SwitchCustomLAF customLAF;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K_Switch)
};


/*
juce::TextButton myButton;

//==============================================================================
//=== 1. COMPORTEMENT & CONFIGURATION DE BASE
//==============================================================================
myButton.setButtonText("MON BOUTON");                  // Texte affiché par défaut
myButton.setClickingTogglesState(true);               // Permet de basculer l'état (true/false) au clic
myButton.setToggleState(false, juce::dontSendNotification); // État initial (false = off, true = on)

// Comportement au clic (si non-toggle, simple déclencheur d'action)
myButton.onClick = [this]() {
	// Code exécuté au clic
	};

// Écouter le changement d'état (bascule on/off)
myButton.onStateChange = [this]() {
	// if (myButton.getToggleState()) { ... }
	};


//==============================================================================
//=== 2. COULEURS SELON L'ÉTAT (FALSE vs TRUE)
//==============================================================================
// Dans JUCE, le TextButton gère plusieurs identifiants de couleur (ColourIds).
// On peut lui injecter des couleurs différentes pour l'état normal (false) et l'état activé (true).

// --- ÉTAT INACTIF / FALSE (OFF) ---
myButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff222222)); // Fond du bouton (false)
myButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffaaaaaa)); // Couleur du texte (false)

// --- ÉTAT ACTIF / TRUE (ON) ---
myButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffd4b26f)); // Fond du bouton (true - ex: doré/actif)
myButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xff000000)); // Couleur du texte (true - ex: noir sur fond clair)

// --- CONTOURS & HOVER (GLOBAL) ---
myButton.setColour(juce::TextButton::outlineColourId, juce::Colour(0xff444444)); // Couleur de la bordure
myButton.setColour(juce::ComboBox::focusedOutlineColourId, juce::Colours::transparentBlack); // Évite un contour de focus moche


//==============================================================================
//=== 3. TYPOGRAPHIE & APPARENCE VISUELLE
//==============================================================================
// Pour changer la police d'un TextButton, on passe par son LookAndFeel ou directement via une police personnalisée
// (Astuce : on peut aussi utiliser un Label par-dessus si on veut un contrôle total du texte, mais le bouton gère l'autofit).

// Modifier les coins arrondis (nécessite de surcharger le LookAndFeel ou d'utiliser les propriétés graphiques du LAF V4)
// Ex dans un LookAndFeel custom :
// void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override { ... }


//==============================================================================
//=== 4. EXEMPLE PRATIQUE : BOUTON DE TYPE "COMMAND VIEW" (ON/OFF)
//==============================================================================
/*
juce::TextButton commandViewBtn;

// Config
commandViewBtn.setButtonText("COMMANDES");
commandViewBtn.setClickingTogglesState(true);

// Couleurs dynamiques False (éteumé/sombre) vs True (allumé/actif)
commandViewBtn.setColour(juce::TextButton::buttonColourId,   juce::Colour(0xff1e1e1e)); // Gris sombre (False)
commandViewBtn.setColour(juce::TextButton::textColourOffId,  juce::Colour(0xff888888)); // Texte grisé (False)

commandViewBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff29b6f6)); // Bleu vif (True)
commandViewBtn.setColour(juce::TextButton::textColourOnId,   juce::Colour(0xff000000)); // Texte noir (True)

commandViewBtn.setColour(juce::TextButton::outlineColourId,  juce::Colour(0xff333333)); // Bordure discrète

// Action liée à l'APVTS ou à l'état
commandViewBtn.onClick = [this, &commandViewBtn]() {
	bool isEnabled = commandViewBtn.getToggleState();
	// Faire quelque chose avec isEnabled (ex: setCommandViewState(isEnabled));
};
*/