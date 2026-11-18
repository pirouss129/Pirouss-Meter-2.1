#include "LUBargraph.h"

LUBargraph::LUBargraph() {}

void LUBargraph::setLevels(float luMomentary, float luShortTerm, float lra, float luIntegrated)
{
    if (luM != luMomentary || luS != luShortTerm || lraValue != lra || luI != luIntegrated)
    {
        luM = luMomentary;
        luS = luShortTerm;
        lraValue = lra;
        luI = luIntegrated;
        repaint();
    }
}

// 🔵🟡🔴 Couleur dynamique LED par LED selon le niveau du segment
juce::Colour LUBargraph::getSegmentColour(float segDb, bool isMomentary) const
{
    float dev = segDb - targetLUFS;

    // Seuil de tolérance (+/- 3 LU pour M, +/- 1 LU pour S)
    float yellowThreshold = isMomentary ? 3.0f : 1.0f;

    if (dev > yellowThreshold)   return juce::Colour(0xffd50000); // Rouge Crimson
    if (dev >= -yellowThreshold) return juce::Colour(0xfffbc02d); // Jaune Ambre
    return juce::Colour(0xff0288d1);                     // Bleu Mat
}

// 🟣 Couleur pour les LEDs du LRA
juce::Colour LUBargraph::getLraSegmentColour(float segLu) const
{
    if (segLu > 12.0f) return juce::Colour(0xffab47bc);
    if (segLu > 6.0f)  return juce::Colour(0xff8e24aa);
    return juce::Colour(0xff6a1b9a);
}

// 🟢 Dessin d'un bargraphe LUM / LUS avec seuils dynamiques par LED
void LUBargraph::drawSingleLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float value, bool isMomentary)
{
    const float minDb = isAbsolute ? -50.0f : (targetLUFS - 18.0f);
    const float maxDb = isAbsolute ? 0.0f : (targetLUFS + 6.0f);

    const float totalH = bounds.getHeight();
    const float segH = totalH / static_cast<float>(numSegments);
    const float gap = 1.0f;

    int activeIdx = -1;
    if (value > minDb)
    {
        float norm = (value - minDb) / (maxDb - minDb);
        activeIdx = juce::jlimit(0, numSegments - 1, (int)(norm * numSegments));
    }

    for (int i = 0; i < numSegments; ++i)
    {
        float y = bounds.getBottom() - ((i + 1) * segH);
        auto segRect = juce::Rectangle<float>(bounds.getX(), y + (gap * 0.5f), bounds.getWidth(), segH - gap);

        float segDb = minDb + ((i + 0.5f) / static_cast<float>(numSegments)) * (maxDb - minDb);
        juce::Colour c = getSegmentColour(segDb, isMomentary);

        if (i <= activeIdx)
        {
            g.setColour(c);
            g.fillRect(segRect);

            g.setColour(juce::Colours::white.withAlpha(0.35f));
            g.fillRect(segRect.reduced(1.5f));
        }
        else
        {
            g.setColour(c.darker(0.9f).withAlpha(0.12f));
            g.fillRect(segRect);
        }
    }
}

// 🟣 Dessin du bargraphe LRA
void LUBargraph::drawLraLedBar(juce::Graphics& g, juce::Rectangle<float> bounds, float lraVal, float integratedLu)
{
    const float minDb = isAbsolute ? -50.0f : (targetLUFS - 18.0f);
    const float maxDb = isAbsolute ? 0.0f : (targetLUFS + 6.0f);

    const float totalH = bounds.getHeight();
    const float segH = totalH / static_cast<float>(numSegments);
    const float gap = 1.0f;

    float centerLu = (integratedLu > minDb) ? integratedLu : luS;
    float safeLra = (lraVal > 0.2f) ? lraVal : 1.0f;

    float lraBottom = centerLu - (safeLra * 0.5f);
    float lraTop = centerLu + (safeLra * 0.5f);

    auto dbToIdx = [minDb, maxDb, numSegs = numSegments](float db) -> int {
        if (db <= minDb) return -1;
        if (db >= maxDb) return numSegs - 1;
        return juce::jlimit(0, numSegs - 1, (int)(((db - minDb) / (maxDb - minDb)) * numSegs));
        };

    int startIdx = dbToIdx(lraBottom);
    int endIdx = dbToIdx(lraTop);

    bool hasSignal = (centerLu > minDb);

    for (int i = 0; i < numSegments; ++i)
    {
        float y = bounds.getBottom() - ((i + 1) * segH);
        auto segRect = juce::Rectangle<float>(bounds.getX(), y + (gap * 0.5f), bounds.getWidth(), segH - gap);

        juce::Colour c = juce::Colour(0xff8e24aa);

        bool isInLraRange = hasSignal && (i >= startIdx && i <= endIdx);

        if (isInLraRange)
        {
            g.setColour(c);
            g.fillRect(segRect);

            g.setColour(juce::Colours::white.withAlpha(0.35f));
            g.fillRect(segRect.reduced(1.5f));
        }
        else
        {
            g.setColour(c.darker(0.9f).withAlpha(0.12f));
            g.fillRect(segRect);
        }
    }
}

void LUBargraph::paint(juce::Graphics& g)
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

    juce::DropShadow outerShadow(juce::Colours::black.withAlpha(0.7f), 6, juce::Point<int>(0, 2));
    outerShadow.drawForPath(g, framePath);

    g.setColour(juce::Colour(0xff121212));
    g.fillPath(framePath);

    g.setColour(borderColour);
    g.strokePath(framePath, juce::PathStrokeType(0.5f));

    // -------------------------------------------------------------------------
    // 3. DÉCOUPAGE DE LA ZONE INTÉRIEURE
    // -------------------------------------------------------------------------
    auto innerArea = area.reduced(6.0f);

    auto labelArea = innerArea.removeFromBottom(16.0f);

    innerArea.removeFromTop(4.0f);
    innerArea.removeFromBottom(4.0f);

    // -------------------------------------------------------------------------
    // 4. RÉPARTITION HORIZONTALE DES COLONNES : [ LUM ] [ LUS ] [ SCALE ] [ LRA ]
    // -------------------------------------------------------------------------
    const float colWidth = innerArea.getWidth() / 4.0f;

    auto lumBounds = innerArea.removeFromLeft(colWidth).reduced(2.0f, 0.0f);
    auto lusBounds = innerArea.removeFromLeft(colWidth).reduced(2.0f, 0.0f);
    auto scaleBounds = innerArea.removeFromLeft(colWidth);
    auto lraBounds = innerArea.reduced(2.0f, 0.0f);

    const float minDb = isAbsolute ? -50.0f : (targetLUFS - 18.0f);
    const float maxDb = isAbsolute ? 0.0f : (targetLUFS + 6.0f);

    // -------------------------------------------------------------------------
    // 5. DESSIN DES BARGRAPHES (LUM, LUS, LRA)
    // -------------------------------------------------------------------------
    drawSingleLedBar(g, lumBounds, luM, true);
    drawSingleLedBar(g, lusBounds, luS, false);
    drawLraLedBar(g, lraBounds, lraValue, luI);

    // -------------------------------------------------------------------------
    // 6. DESSIN DE L'ÉCHELLE CENTRALE
    // -------------------------------------------------------------------------
    g.setFont(juce::FontOptions(9.0f));

    if (isAbsolute)
    {
        // --- MODE ABSOLU (LUFS) ---
        const std::vector<float> lufsTicks = { 0.0f, -6.0f, -12.0f, -15.0f, -18.0f, -23.0f, -30.0f, -40.0f };
        for (float tickVal : lufsTicks)
        {
            float norm = (tickVal - minDb) / (maxDb - minDb);
            float y = scaleBounds.getBottom() - (norm * scaleBounds.getHeight());

            bool isTarget = (std::abs(tickVal - targetLUFS) < 0.5f);
            g.setColour(isTarget ? juce::Colour(0xffffea00) : juce::Colours::grey);
            g.drawText(juce::String((int)tickVal), scaleBounds.withY(y - 6.0f).withHeight(12.0f), juce::Justification::centred, false);
        }
    }
    else
    {
        // --- MODE RELATIF (LU par rapport au Target) ---
        const std::vector<float> luTicks = { 6.0f, 3.0f, 1.0f, 0.0f, -1.0f, -3.0f, -6.0f, -12.0f, -18.0f };
        for (float luVal : luTicks)
        {
            float absoluteLUFS = targetLUFS + luVal;
            if (absoluteLUFS > maxDb || absoluteLUFS < minDb) continue;

            float norm = (absoluteLUFS - minDb) / (maxDb - minDb);
            float y = scaleBounds.getBottom() - (norm * scaleBounds.getHeight());

            bool isZeroLU = (luVal == 0.0f);
            g.setColour(isZeroLU ? juce::Colour(0xffffea00) : juce::Colours::grey);

            juce::String txt = (luVal > 0.0f) ? "+" + juce::String((int)luVal) : juce::String((int)luVal);
            g.drawText(txt, scaleBounds.withY(y - 6.0f).withHeight(12.0f), juce::Justification::centred, false);
        }
    }

    // -------------------------------------------------------------------------
    // 7. LABELS DES CANAUX / MESURES EN BAS ("M", "S", "LU/LUFS", "LRA")
    // -------------------------------------------------------------------------
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colours::lightgrey);

    float labelWidth = labelArea.getWidth() / 4.0f;
    g.drawText("M", labelArea.removeFromLeft(labelWidth), juce::Justification::centred);
    g.drawText("S", labelArea.removeFromLeft(labelWidth), juce::Justification::centred);
    g.drawText(isAbsolute ? "LUFS" : "LU", labelArea.removeFromLeft(labelWidth), juce::Justification::centred);
    g.drawText("LRA", labelArea, juce::Justification::centred);
}

void LUBargraph::resized() {}