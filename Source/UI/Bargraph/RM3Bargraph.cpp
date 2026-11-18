#include "RM3Bargraph.h"

RM3Bargraph::RM3Bargraph() {}

void RM3Bargraph::setLevels(float rmsShort, float rmsMom)
{
    if (rmsS != rmsShort || rmsM != rmsMom)
    {
        rmsS = rmsShort;
        rmsM = rmsMom;
        repaint();
    }
}

// 🟢 CALCUL DES COULEURS DYNAMIQUES PAR RAPPORT AU ZERO K-SYSTEM (kRefDb)
juce::Colour RM3Bargraph::getSegmentColour(float db) const
{
    // dbK représente la déviation par rapport à la référence (ex: -18 dB devient 0 dBK)
    float dbK = db - kRefDb;

    // 🔴 Rouge foncé / Crimson mat (Zone critique)
    if (dbK > 3.0f)     return juce::Colour(0xffd50000);

    // 🟠 Amber / Warm Orange (Zone d'avertissement)
    if (dbK >= 0.0f)    return juce::Colour(0xffe65100);

    // 🟡 Jaune Ambre / Moutarde feutré (Zone cible RMS)
    if (dbK >= -6.0f)   return juce::Colour(0xfffbc02d);

    // 🟢 Vert Bouteille / Deep Forest (Zone sous le niveau cible)
    return juce::Colour(0xff1b5e20);
}
/*
void RM3Bargraph::drawSquareLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float valBarDb, float valLedDb)
{
    const float totalH = bounds.getHeight();
    const float segH = totalH / static_cast<float>(numSegments);
    const float gap = 1.5f;

    auto dbToIdx = [this](float db) -> int {
        if (db <= minDb) return -1;
        if (db >= maxDb) return numSegments - 1;
        return juce::jlimit(0, numSegments - 1, (int)(((db - minDb) / (maxDb - minDb)) * numSegments));
        };

    int barIdx = dbToIdx(valBarDb);
    int ledIdx = dbToIdx(valLedDb);

    for (int i = 0; i < numSegments; ++i)
    {
        float y = bounds.getBottom() - ((i + 1) * segH);

        float ledSize = std::min(bounds.getWidth(), segH - gap);
        float xOffset = (bounds.getWidth() - ledSize) * 0.5f;
        auto segRect = juce::Rectangle<float>(bounds.getX() + xOffset, y + (gap * 0.5f), ledSize, ledSize);

        float segDb = minDb + ((i + 0.5f) / (float)numSegments) * (maxDb - minDb);
        juce::Colour c = getSegmentColour(segDb);

        bool isBarOn = (i <= barIdx);
        bool isLedOn = (i == ledIdx);

        if (isBarOn || isLedOn)
        {
            g.setColour(c);
            g.fillRect(segRect);

            g.setColour(juce::Colours::white.withAlpha(isLedOn ? 0.75f : 0.35f));
            g.fillRect(segRect.reduced(ledSize * 0.25f));
        }
        else
        {
            g.setColour(c.darker(0.9f).withAlpha(0.15f));
            g.fillRect(segRect);
        }
    }
}
*/
void RM3Bargraph::drawSquareLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float valBarDb, float valLedDb)
{
    const float totalH = bounds.getHeight();
    const float segH = totalH / static_cast<float>(numSegments);
    const float gap = 1.5f;

    auto dbToIdx = [this](float db) -> int {
        if (db <= minDb) return -1;
        if (db >= maxDb) return numSegments - 1;
        return juce::jlimit(0, numSegments - 1, (int)(((db - minDb) / (maxDb - minDb)) * numSegments));
        };

    int barIdx = dbToIdx(valBarDb); // Niveau RMS (bloc continu)
    int ledIdx = dbToIdx(valLedDb); // Niveau Peak (diode volante)

    for (int i = 0; i < numSegments; ++i)
    {
        float y = bounds.getBottom() - ((i + 1) * segH);

        float ledSize = std::min(bounds.getWidth(), segH - gap);
        float xOffset = (bounds.getWidth() - ledSize) * 0.5f;
        auto segRect = juce::Rectangle<float>(bounds.getX() + xOffset, y + (gap * 0.5f), ledSize, ledSize);

        float segDb = minDb + ((i + 0.5f) / (float)numSegments) * (maxDb - minDb);
        juce::Colour c = getSegmentColour(segDb);

        bool isBarOn = (i <= barIdx);
        bool isLedOn = (i == ledIdx);

        // 1. DESSIN DU CORPS RMS (Eteint ou Allumé)
        if (isBarOn)
        {
            g.setColour(c);
            g.fillRect(segRect);

            // Petit relief interne feutré pour le bloc RMS
            g.setColour(juce::Colours::white.withAlpha(0.20f));
            g.fillRect(segRect.reduced(ledSize * 0.25f));
        }
        else
        {
            // Segment RMS éteint (fond sombre)
            g.setColour(c.darker(0.9f).withAlpha(0.12f));
            g.fillRect(segRect);
        }

        // 2. DESSIN EXCLUSIF ET FLAMBOYANT DE LA DIODE PEAK
        if (isLedOn)
        {
            // Fond blanc pur très brillant
            g.setColour(juce::Colours::palevioletred.withAlpha(0.5f));
            g.fillRect(segRect);

            // Trait horizontal intense au centre pour l'effet "Filament hardware"
            g.setColour(juce::Colours::red);
            g.fillRect(segRect.reduced(0.0f, ledSize * 0.3f));
        }
    }
}

void RM3Bargraph::paint(juce::Graphics& g)
{
    // -------------------------------------------------------------------------
    // 1. DÉFINITION DE LA ZONE GLOBALE DU MODULE
    // -------------------------------------------------------------------------
    auto area = getLocalBounds().toFloat().reduced(3.0f);

    // -------------------------------------------------------------------------
    // 2. FOND DU MODULE AVEC EFFET HARDWARE ENCASTRÉ
    // -------------------------------------------------------------------------
    const float cornerRadius = 8.0f;

    juce::Path framePath;
    framePath.addRoundedRectangle(area, cornerRadius);

    // Ombre sous le châssis
    juce::DropShadow outerShadow(juce::Colours::black.withAlpha(1.0f), 6, juce::Point<int>(0, 2));
    outerShadow.drawForPath(g, framePath);

    // Fond du boîtier
    g.setColour(juce::Colour(0xff121212));
    g.fillPath(framePath);

    // Bordure métallique
    g.setColour(borderColour);
    g.strokePath(framePath, juce::PathStrokeType(0.5f));

    // -------------------------------------------------------------------------
    // 3. DÉCOUPAGE DE LA ZONE INTÉRIEURE
    // -------------------------------------------------------------------------
    auto innerArea = area.reduced(4.0f);
    auto labelArea = innerArea.removeFromBottom(16.0f);

    const float topPadding = 4.0f;
    const float bottomPadding = 4.0f;

    innerArea.removeFromTop(topPadding);
    innerArea.removeFromBottom(bottomPadding);

    // -------------------------------------------------------------------------
    // 4. RÉPARTITION HORIZONTALE
    // -------------------------------------------------------------------------
    const float scaleWidth = 22.0f;
    juce::Rectangle<float> barBounds;
    juce::Rectangle<float> scaleBounds;

    if (scalePos == ScalePosition::Left)
    {
        scaleBounds = innerArea.removeFromLeft(scaleWidth);
        barBounds = innerArea;
    }
    else if (scalePos == ScalePosition::Right)
    {
        scaleBounds = innerArea.removeFromRight(scaleWidth);
        barBounds = innerArea;
    }
    else
    {
        barBounds = innerArea;
    }

    // -------------------------------------------------------------------------
    // 5. DESSIN DU BARGRAPHE
    // -------------------------------------------------------------------------
    drawSquareLedBar(g, barBounds, rmsS, rmsM);

    // -------------------------------------------------------------------------
    // 6. DESSIN DE L'ÉCHELLE DYNAMIQUE
    // -------------------------------------------------------------------------
    if (scalePos != ScalePosition::None)
    {
        g.setFont(juce::FontOptions(9.0f));

        if (isAbsolute)
        {
            // --- MODE ABSOLU (dBFS) ---
            const std::vector<float> fsTicks = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -30.0f, -40.0f };

            for (float dbFS : fsTicks)
            {
                float norm = (dbFS - minDb) / (maxDb - minDb);
                float y = barBounds.getBottom() - (norm * barBounds.getHeight());

                g.setColour(dbFS == 0.0f ? juce::Colours::white : juce::Colours::grey);
                g.drawText(juce::String((int)dbFS), scaleBounds.withY(y - 6.0f).withHeight(12.0f), juce::Justification::centred, false);
            }
        }
        else
        {
            // --- MODE RELATIF (K-System) ---
            const std::vector<float> kValuesToDraw = { 20.0f, 18.0f, 14.0f, 12.0f, 6.0f, 3.0f, 0.0f, -6.0f, -12.0f, -22.0f };

            for (float kVal : kValuesToDraw)
            {
                float dbFS = kVal + kRefDb;
                if (dbFS > maxDb || dbFS < minDb) continue;

                float norm = (dbFS - minDb) / (maxDb - minDb);
                float y = barBounds.getBottom() - (norm * barBounds.getHeight());

                bool isZeroK = (kVal == 0.0f);
                g.setColour(isZeroK ? juce::Colour(0xffffea00) : juce::Colours::grey);

                juce::String txt = (kVal > 0.0f) ? "+" + juce::String((int)kVal) : juce::String((int)kVal);
                g.drawText(txt, scaleBounds.withY(y - 6.0f).withHeight(12.0f), juce::Justification::centred, false);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 7. LABEL DU CANAL
    // -------------------------------------------------------------------------
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText(channelName, labelArea, juce::Justification::centred, false);
}

void RM3Bargraph::resized()
{
}