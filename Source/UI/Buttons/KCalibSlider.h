#pragma once
#include <JuceHeader.h>

class K_CalibSlider : public juce::Component
{
public:
	juce::Slider slider;

		K_CalibSlider(juce::Colour color=juce::Colours::red)
	{
		// --- CONFIG SLIDER ---
		slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, color); // On stocke la couleur de l'aiguille du potard dans les couleurs natives du slider !)
        slider.setColour(juce::Slider::backgroundColourId, juce::Colours::yellow);

		slider.setTextBoxStyle(juce::Slider::TextBoxLeft, true, 50, 20);	// boite à gauche, pas de clic à l'intérieur, largeur, hauteur
		slider.setColour(juce::Slider::textBoxTextColourId, color);        // Texte de la valeur
		slider.setColour(juce::Slider::textBoxOutlineColourId, color);     // Cadre autour de la valeur
		slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff121212));  // Fond de la boîte de valeur

		
		

		slider.setDoubleClickReturnValue(true, 0.0);

		slider.setLookAndFeel(&customLAF);

		addAndMakeVisible(slider);
	}
		~K_CalibSlider() override
		{
			slider.setLookAndFeel(nullptr);
		}

	void resized() override
	{
		auto bounds = getLocalBounds();
		slider.setBounds(bounds);
	}
		
private:
    // LookAndFeel interne et propre à ce composant
    class RotaryCustomLAF : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
            juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(4.0f);
            auto radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();

            auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // 0. Vraie ombre portée circulaire
            juce::Path shadowPath;
            shadowPath.addEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

            juce::DropShadow(juce::Colours::black, 16, juce::Point<int>(3, 3)).drawForPath(g, shadowPath);

            /*
            // 1. Fond du potard
            g.setColour(slider.findColour(juce::Slider::backgroundColourId)); // On utilise la couleur définie pour le fond du potard
            g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
            */

            // 1. Fond du potard avec un dégradé (Lumière haut-gauche -> Bas-droite)
            juce::Rectangle<float> dialRect(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

            juce::ColourGradient metalGradient(
                //juce::Colour(0xff555555), dialRect.getTopLeft(),      // Gris clair en haut-gauche (reçoit la lumière)
                //juce::Colour(0xff1a1a1a), dialRect.getBottomRight(),   // Gris très sombre / noir en bas-droite (zone d'ombre)
                juce::Colours::darkgrey, dialRect.getTopLeft(),      // Gris clair en haut-gauche (reçoit la lumière)
                juce::Colours::black, dialRect.getBottomRight(),   // Gris très sombre / noir en bas-droite (zone d'ombre)
                false // Dégradé linéaire
            );

            g.setGradientFill(metalGradient);
            g.fillEllipse(dialRect);

            // 2. Contour
            g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId)); // On utilise la couleur définie pour le contour du potard
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

            // 3. Aiguille
            /*
            juce::Path pointer;
            auto pointerLength = radius * 0.75f;
            auto pointerThickness = 3.0f;

            pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
            pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.fillPath(pointer);
            */

            // 3. Pointeur rond (pastille)
            auto pointerDistance = radius * 0.65f; // Distance du centre (0 = au milieu, 1 = tout au bord)
            auto dotRadius = 3.0f;                 // Taille du point rond

            // On calcule la position exacte (X, Y) du point en fonction de l'angle
            auto dotX = centre.x + std::sin(angle) * pointerDistance;
            auto dotY = centre.y - std::cos(angle) * pointerDistance;

            // On dessine le petit cercle
            g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId)); // On utilise la couleur définie pour le contour du potard pour le point
            g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
        }
    };

    RotaryCustomLAF customLAF; // Instance du LookAndFeel embarquée
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K_CalibSlider)

};