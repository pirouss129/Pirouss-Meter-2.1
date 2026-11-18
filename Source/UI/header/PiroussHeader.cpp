#include "PiroussHeader.h"

// -----------------------------------------------------------------------------
// CONSTRUCTEUR
// -----------------------------------------------------------------------------
PiroussHeader::PiroussHeader(const juce::String& pluginName, const juce::String& pluginVersion, const juce::Colour themeColor)
    : name(pluginName), version(pluginVersion), color(themeColor)
{
}

// -----------------------------------------------------------------------------
// MÉTHODE PAINT()
// -----------------------------------------------------------------------------
void PiroussHeader::paint(juce::Graphics& g)
{
    // 1. DÉFINITION DE LA ZONE DU HEADER
    // Marge pour laisser respirer l'ombre portée dans les limites du composant
    auto bounds = getLocalBounds().toFloat();
    //g.fillAll(juce::Colours::red); // Fond rouge pour le test

    float margeX = 4.0f;
    float margeY = 4.0f;
    float cornerRadius = 5.0f;

    auto zoneChrome = bounds.reduced(margeX, margeY);

    // 2. CRÉATION DU CHEMIN (PATH) DU CHÂSSIS HEADER
    juce::Path framePath;
    framePath.addRoundedRectangle(zoneChrome, cornerRadius);

    // A. Ombre sous le header (Drop Shadow pour effet d'encastrement)
    juce::DropShadow outerShadow(juce::Colours::black.withAlpha(0.6f), 5, juce::Point<int>(0, 2));
    outerShadow.drawForPath(g, framePath);

    // B. Remplissage avec dégradé métallique / thème
    juce::ColourGradient chromeGrad(
        color.withAlpha(0.45f), 0.0f, zoneChrome.getY(),
        juce::Colours::black.withAlpha(0.85f), 0.0f, zoneChrome.getBottom(),
        false
    );
    g.setGradientFill(chromeGrad);
    g.fillPath(framePath);

    // C. Contour biseauté avec la couleur du thème
    g.setColour(color.withAlpha(0.9f));
    g.strokePath(framePath, juce::PathStrokeType(1.2f));

    // 3. LOGO JUCE (SI PRÉSENT DANS LES RESSOURCES BINAIRES)
    juce::Image juceLogo = juce::ImageCache::getFromMemory(BinaryData::juce_logo_png, BinaryData::juce_logo_pngSize);
    if (juceLogo.isValid())
    {
        auto logoZone = zoneChrome.removeFromLeft(25.0f).reduced(4.0f);
        g.setOpacity(1.0f);
        g.drawImageWithin(juceLogo,
            (int)logoZone.getX(), (int)logoZone.getY(),
            (int)logoZone.getWidth(), (int)logoZone.getHeight(),
            juce::RectanglePlacement::centred);
    }

    // 4. TITRE ET VERSION
    g.setColour(juce::Colours::white.withAlpha(0.95f));
    g.setFont(juce::FontOptions(13.0f, juce::Font::italic));

    juce::String fullText = name + " v" + version;
    g.drawText(fullText, bounds, juce::Justification::centred, true);
}