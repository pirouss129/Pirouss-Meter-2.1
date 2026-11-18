#include "DigitalDisplay.h"

DigitalDisplay::DigitalDisplay(const juce::String& labelText, juce::Colour valueColour)
    : label(labelText), displayColour(valueColour)
{
}

void DigitalDisplay::setValue(float newValue)
{
    if (value != newValue)
    {
        value = newValue;
        repaint();
    }
}

void DigitalDisplay::setLabel(const juce::String& newLabel)
{
    label = newLabel;
    repaint();
}

void DigitalDisplay::paint(juce::Graphics& g)
{
    // -------------------------------------------------------------------------
    // 1. DÉFINITION DE LA ZONE GLOBALE DU MODULE
    // -------------------------------------------------------------------------
    // Réduction de 4px sur les bords pour laisser baver l'ombre portée
    // sans qu'elle soit tronquée par les limites du composant.
    auto area = getLocalBounds().toFloat().reduced(3.0f);
    //g.fillAll(juce::Colours::yellow); // Fond rouge pour le test

    // -------------------------------------------------------------------------
    // 2. FOND DU MODULE AVEC EFFET HARDWARE ENCASTRÉ
    // -------------------------------------------------------------------------
    const float cornerRadius = 8.0f;

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

    // -------------------------------------------------------------------------
    // 3. DÉCOUPAGE ET ORGANISATION DE LA ZONE D'AFFICHAGE
    // -------------------------------------------------------------------------
    auto contentArea = area.reduced(3.0f);

    // A. Libellé (TP, I, LRA, etc.) en haut
    auto labelArea = contentArea.removeFromTop(12.0f);
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.setColour(juce::Colours::grey);
    g.drawText(label, labelArea, juce::Justification::centred, false);

    // B. Zone réservée à l'afficheur numérique LED 7 segments / Monospace
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(),
        contentArea.getHeight() * 0.75f,
        juce::Font::bold));

    // C. Masque "88.8" feutré en arrière-plan pour le rendu vintage (segments éteints)
    g.setColour(displayColour.withAlpha(0.08f));
    g.drawText("88.8", contentArea, juce::Justification::centred, false);

    // D. Valeur réelle allumée
    g.setColour(displayColour);
    juce::String valStr = (value <= -60.0f) ? "---" : juce::String(value, 1);
    g.drawText(valStr, contentArea, juce::Justification::centred, false);
}

void DigitalDisplay::resized()
{
}