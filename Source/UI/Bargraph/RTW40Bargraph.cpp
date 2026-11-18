#include "RTW40Bargraph.h"

RTW40Bargraph::RTW40Bargraph() {}

void RTW40Bargraph::setLevels(float qppmL, float qppmR, float ppmHoldL, float ppmHoldR)
{
    levelL = qppmL; levelR = qppmR;
    peakL = ppmHoldL; peakR = ppmHoldR;
    repaint();
}

juce::Colour RTW40Bargraph::getSegmentColour(float db) const
{
    if (db >= -5.0f)   return juce::Colour(0xffff1744); // Rouge (0 à -5 dB)
    if (db >= -9.0f)  return juce::Colour(0xffff9100); // Orange (-6 à -9 dB)
    if (db >= -18.0f) return juce::Colour(0xffffea00); // Jaune (-9 à -18 dB)
    return juce::Colour(0xff00e676);                  // Vert (en-dessous de -18 dB)
}

void RTW40Bargraph::drawSingleBar(juce::Graphics& g, juce::Rectangle<float> bounds, float valDb, float holdDb)
{
    const float totalH = bounds.getHeight();
    const float segH = totalH / static_cast<float>(numSegments);
    const float gap = 1.0f;

    // Réduction latérale pour affiner les LED et ne pas coller à l'échelle
    bounds = bounds.reduced(2.0f, 0.0f);

    // Fonction lambda pour convertir une valeur dB en index de segment (LED)
    auto dbToIdx = [this](float db) -> int {
        if (db <= minDb) return -1;
        if (db >= maxDb) return numSegments - 1;
        return juce::jlimit(0, numSegments - 1, (int)(((db - minDb) / (maxDb - minDb)) * numSegments));
        };

    int barIdx = dbToIdx(valDb);
    int holdIdx = dbToIdx(holdDb);

    for (int i = 0; i < numSegments; ++i)
    {
        // Position Y de chaque segment (du bas vers le haut)
        float y = bounds.getBottom() - ((i + 1) * segH);
        auto segRect = juce::Rectangle<float>(bounds.getX(), y + (gap * 0.5f), bounds.getWidth(), segH - gap);

        // Valeur dB représentée par ce segment spécifique
        float segDb = minDb + ((i + 0.5f) / (float)numSegments) * (maxDb - minDb);
        juce::Colour c = getSegmentColour(segDb);

        bool isOn = (i <= barIdx);
        bool isHold = (i == holdIdx);

        if (isOn || isHold)
        {
            // Segment allumé ou Peak Hold
            g.setColour(c);
            g.fillRoundedRectangle(segRect, 2.5f); // Angles des LED arrondis

            // Reflet central brillant sur la LED allumée
            g.setColour(juce::Colours::white.withAlpha(isHold ? 0.75f : 0.25f));
            g.fillRoundedRectangle(segRect.reduced(segRect.getWidth() * 0.2f, segRect.getHeight() * 0.2f), 1.5f);
        }
        else
        {
            // Segment éteint (fond assombri)
            g.setColour(c.darker(0.85f).withAlpha(0.2f));
            g.fillRoundedRectangle(segRect, 2.5f);
        }
    }
}

void RTW40Bargraph::paint(juce::Graphics& g)
{
    // -------------------------------------------------------------------------
    // 1. FOND DE LA FENÊTRE ÉDITEUR (ARRIÈRE-PLAN)
    // -------------------------------------------------------------------------
    //g.fillAll(juce::Colour(0xff0a0a0a));
	//g.fillAll(juce::Colours::green); // Fond rouge pour le test

    // Zone globale du module RTW
	auto area = getLocalBounds().toFloat().reduced(3.0f); // pas de marge pour le moment, mais on peut ajuster si nécessaire

    // -------------------------------------------------------------------------
    // 2. FOND DU MODULE AVEC EFFET HARDWARE ENCASTRÉ
    // -------------------------------------------------------------------------
    const float cornerRadius = 8.0f;
    juce::Path framePath;
    framePath.addRoundedRectangle(area, cornerRadius);

    // A. Ombre sous le châssis (Relief extérieur)
    juce::DropShadow outerShadow(juce::Colours::black.withAlpha(0.7f), 6, juce::Point<int>(0, 2));
    outerShadow.drawForPath(g, framePath);

    // B. Remplissage du fond du boîtier (Plastique/Métal anthracite)
    g.setColour(juce::Colour(0xff121212));
    g.fillPath(framePath);

    // C. Contour discret (Bordure métallique)
    g.setColour(borderColour);
    g.strokePath(framePath, juce::PathStrokeType(0.5f)); // 🟢 strokePath pour dessiner le contour

    // -------------------------------------------------------------------------
    // 3. DÉCOUPAGE DE LA ZONE INTÉRIEURE (MARGES ET PADDING DE RESPIRATION)
    // -------------------------------------------------------------------------
    // Marge interne pour détacher les éléments du bord du cadre
    auto innerArea = area.reduced(4.0f);

    // A. Zone des labels canaux bas ("L" et "R")
    auto labelArea = innerArea.removeFromBottom(16.0f);

    // B. PADDING EN HAUT ET EN BAS (Pour donner de l'air au "0" et au bas des LED)
    const float topPadding = 4.0f;   // 🟢 Air au-dessus du Zéro dBFS
    const float bottomPadding = 4.0f; // Marge au-dessus des labels L/R

    innerArea.removeFromTop(topPadding);
    innerArea.removeFromBottom(bottomPadding);

    // -------------------------------------------------------------------------
    // 4. RÉPARTITION HORIZONTALE : BARRE L - ÉCHELLE - BARRE R
    // -------------------------------------------------------------------------
    const float centerScaleWidth = 26.0f;
    const float barWidth = (innerArea.getWidth() - centerScaleWidth) * 0.5f;

    auto leftBarBounds = innerArea.removeFromLeft(barWidth);
    auto scaleBounds = innerArea.removeFromLeft(centerScaleWidth);
    auto rightBarBounds = innerArea; // Ce qu'il reste à droite pour le canal R

    // -------------------------------------------------------------------------
    // 5. DESSIN DES BARGRAPHES STEREO (LEDS L & R)
    // -------------------------------------------------------------------------
    drawSingleBar(g, leftBarBounds, levelL, peakL);
    drawSingleBar(g, rightBarBounds, levelR, peakR);

    // -------------------------------------------------------------------------
// 6. DESSIN DE L'ÉCHELLE CENTRALE (GRADUATIONS DB)
// -------------------------------------------------------------------------
    g.setFont(juce::FontOptions(9.0f));
    const std::vector<float> ticks = { 0.0f, -3.0f, -6.0f, -9.0f, -12.0f, -18.0f, -24.0f, -30.0f, -40.0f };

    for (float db : ticks)
    {
        // Normalisation de la valeur dB entre 0.0f et 1.0f
        float norm = (db - minDb) / (maxDb - minDb);

        // Calcul Y basé sur la zone 'leftBarBounds' (qui inclut le topPadding)
        float y = leftBarBounds.getBottom() - (norm * leftBarBounds.getHeight());

        // 🟡 Blanc pour 0 dB, Jaune Ambre pour -9 dB, Gris pour le reste
        if (db == 0.0f)
            g.setColour(juce::Colours::white);
        else if (db == -9.0f)
            g.setColour(juce::Colour(0xfffbc02d)); // Jaune RM3
        else
            g.setColour(juce::Colours::grey);

        juce::String txt = juce::String((int)db);

        // Centrage du texte de l'échelle entre les deux barres
        g.drawText(txt, scaleBounds.withY(y - 6.0f).withHeight(12.0f), juce::Justification::centred, false);
    }

    // -------------------------------------------------------------------------
    // 7. DESSIN DES LABELS DE CANAUX ("L" ET "R")
    // -------------------------------------------------------------------------
    g.setFont(juce::FontOptions(11.0f));
    g.setColour(juce::Colours::lightgrey);

    // Alignés pile sous chaque colonne de VU-mètre
    g.drawText("L", labelArea.withX(leftBarBounds.getX()).withWidth(leftBarBounds.getWidth()), juce::Justification::centred);
    g.drawText("R", labelArea.withX(rightBarBounds.getX()).withWidth(rightBarBounds.getWidth()), juce::Justification::centred);
}

void RTW40Bargraph::resized()
{
    // Réservé pour le layout des composants enfants si nécessaire
}