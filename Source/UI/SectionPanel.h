#pragma once

#include <JuceHeader.h>

class SectionPanel : public juce::Component
{
public:
    SectionPanel() = default;

    // Méthode pour changer la couleur de bordure
    juce::Colour borderColour = juce::Colours::ghostwhite.withAlpha(0.5f); // Couleur par défaut de la bordure
    void setBorderColour(const juce::Colour& newColour)
    {
        borderColour = newColour;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // -------------------------------------------------------------------------
        // 1. DÉFINITION DE LA ZONE GLOBALE DU PANNEAU
        // -------------------------------------------------------------------------
        // Réduction de 4px sur les bords pour éviter que l'ombre portée
        // ne soit tronquée par les limites du composant.
        auto area = getLocalBounds().toFloat().reduced(3.0f);
        //g.fillAll(juce::Colours::pink); // Fond rouge pour le test

        // -------------------------------------------------------------------------
        // 2. FOND DU PANNEAU AVEC EFFET HARDWARE ENCASTRÉ
        // -------------------------------------------------------------------------
        // Création du chemin (Path) pour la forme arrondie du châssis
        juce::Path framePath;
        framePath.addRoundedRectangle(area, cornerRadius);

        // A. Ombre sous le châssis (Relief extérieur / Drop Shadow)
        juce::DropShadow outerShadow(juce::Colours::black.withAlpha(0.7f), 6, juce::Point<int>(0, 2));
        outerShadow.drawForPath(g, framePath);

        // B. Remplissage du fond du boîtier (Plastique/Métal anthracite)
        g.setColour(juce::Colour(0xff121212));
        g.fillPath(framePath);

        // C. Contour discret (Bordure métallique avec biseau)
        g.setColour(borderColour);
        g.strokePath(framePath, juce::PathStrokeType(0.5f));
    }

private:
    float cornerRadius = 8.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SectionPanel)
};