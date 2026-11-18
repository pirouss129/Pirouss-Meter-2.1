/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SectionPanel.h"
#include "UI/Bargraph/DigitalDisplay.h"
#include "UI/Bargraph/RM3Bargraph.h"
#include "UI/Bargraph/RTW40Bargraph.h"
#include "UI/Bargraph/LUBargraph.h"
#include "UI/header/PiroussHeader.h"
#include "UI/Buttons/KSwitch.h"
#include "UI/Buttons/KCalibSlider.h"
#include "UI/Buttons/LuSwitch.h"
#include "UI/Buttons/LuCalibSlider.h"

//==============================================================================
/**
*/
class PiroussMeter2_1AudioProcessorEditor  : public juce::AudioProcessorEditor, 
    public juce::Timer,  // hériter de Timer
    public juce::AudioProcessorValueTreeState::Listener //hériter de APVTS LISTENER
{
public:
    PiroussMeter2_1AudioProcessorEditor (PiroussMeter2_1AudioProcessor&);
    ~PiroussMeter2_1AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void timerCallback() override;  // déclaration de la méthode timerCallback
    void resized() override;

    void mouseDown(const juce::MouseEvent& event) override; // CLIC DROIT
    void mouseDoubleClick(const juce::MouseEvent& event);

    // méthode appelée automatiquement par l'APVTS quand un paramètre change
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PiroussMeter2_1AudioProcessor& audioProcessor;

    //==============================================================================
	// --- Méthodes et variables pour gérer les modes d'affichage ---
	enum class peakOrLuViewMode //type d'énumération pour le mode d'affichage Peak ou LUFS
	{
		Both = 0, // premier mode d'affichage
		PeakOnly, //second mode qui est le 2 par déduction du code, mais on peut le mettre explicitement
        LuOnly
	};
	peakOrLuViewMode currentPeakOrLuViewMode = peakOrLuViewMode::Both; //variable pour stocker le mode d'affichage actuel, initialisé à Both
    peakOrLuViewMode prevPeakOrLuViewMode = peakOrLuViewMode::Both; // variable pour stocker l'ancien affichage, utile pour calcul dans updatePluginSize
    void setPeakOrLuViewMode(peakOrLuViewMode newMode); // type - variable
    
    bool currentDisplayViewState = true;
    bool prevDisplayViewState = currentDisplayViewState;
	void setDisplayViewState(bool newMode);

    bool currentCommandViewState = true;
    bool prevCommandViewState = currentCommandViewState;
	void setCommandViewState(bool newMode);

    void setElementsVisibility(); // méthode pour déterminer ce qui doit être visible en fonction des booléens modifiés peak or Lu, Display, command

    void updatePluginSize(); // méthode pour mettre à jour la taille du plug

    void setToPiroussConfig();  // méthode pour mettre la config Pirouss

    void resetConfig();

    void paintOverChildren(juce::Graphics& g);

    


    //==============================================================================
	// Déclarations
    
    // Titre et couleurs de base
    const juce::String pluginTitle = "Pirouss Meter";
    const juce::String pluginVersion = JucePlugin_VersionString; // utiliser dans le clic droit
    //const juce::Colour colorTitle = juce::Colour(0xffc8a86b); // Neve/Studer style
    const juce::Colour colorTitle = juce::Colour(0xff29b6f6); // 

    // Couleur de fond du plugin
    const juce::Colour colorBackground = juce::Colour(0xff2a2a2a);
    //const juce::Colour colorBackground = juce::Colours::ghostwhite;

    //==============================================================================
	// AREAS DE L'INTERFACE
	// Zone de l'en-tête
    juce::Rectangle<int> headerArea;

	// Zone Gauche : Peak & RMS
	juce::Rectangle<int> peakRMSArea;
    juce::Rectangle<int>peakRMSVuArea;
    juce::Rectangle<int>RM3LArea;
    juce::Rectangle<int>RTW40Area;
    juce::Rectangle<int>RM3RArea;
    juce::Rectangle<int>tpDisplayArea;
    juce::Rectangle<int>KCommandArea;
    juce::Rectangle<int> KSwitchArea;
    juce::Rectangle<int> KCalibSliderArea;

	// Zone Droite : LU
	juce::Rectangle<int> luArea;
	juce::Rectangle<int> luVuArea;
	juce::Rectangle<int> luDisplayArea;
    juce::Rectangle<int> integratedDisplayArea;
    juce::Rectangle<int> lraDisplayArea;
	juce::Rectangle<int> LCommandArea;
	juce::Rectangle<int> LSwitchArea;
	juce::Rectangle<int> LCalibSliderArea;


    //==============================================================================
    // TAILLES
    static constexpr int headerHeight = 30;
    //static constexpr int BargraphsHeight = 320; // Zone principale des vues VU/LUFS
    static constexpr int displayHeight = 45;
    static constexpr int commandHeight = 45;
    // les bargraphs prennent la taille qui reste

    static constexpr int minWidth = 280;
    static constexpr int minHeight = 400;
    static constexpr int maxWidth = 1200;
    static constexpr int maxHeight = 1200;

    // --- DIMENSIONS PAR DÉFAUT (MODE COMPLET) ---
    static constexpr int defaultWidth = 320;
    static constexpr int defaultHeight = 460; //460
    static constexpr int piroussWidth = 280;
    static constexpr int piroussHeight = 400;
    


    //==============================================================================
	// INSTANCES DES COMPOSANTS
    
    // Instance de l'en-tête du plugin
    PiroussHeader titlePlate{ pluginTitle, "2.1", colorTitle };
    
    // Instances Peak & RMS
	RM3Bargraph rm3LBargraph;
	RTW40Bargraph rtw40Bargraph;
	RM3Bargraph rm3RBargraph;

	// Instance LU
	LUBargraph luBargraph;

    // Afficheurs rétro
    DigitalDisplay tpDisplay{ "TRUE PEAK", juce::Colour(0xff0288d1) };
    DigitalDisplay integratedDisplay{ "INTEGRATED", juce::Colour(0xfffbc02d) };
    DigitalDisplay lraDisplay{ "LRA", juce::Colour(0xff8e24aa) };

	// Instances boutons et sliders
	SectionPanel KCommandPanel; // sert à dessiner une plaque en-dessous des boutons et sliders
	//juce::TextButton KSwitch{ "K abs" };
    K_Switch KSwitch{"ABS", "REL", colorTitle};  // OFF, ON, bgColor ON, bgColor OFF (non déclaré donc couleur par défaut darkgrey
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> kSwitchAttachment; // attachement APVTS
	//juce::Slider KCalibSlider;
    K_CalibSlider KCalibSlider{colorTitle};
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kCalibAttachement; // attachement APVTS

	SectionPanel LCommandPanel; // sert à dessiner une plaque en-dessous des boutons et sliders
	//juce::TextButton LSwitch{ "L abs" };
    L_Switch LSwitch{ "ABS","REL", colorTitle };    // OFF, ON, bgColor ON, bgColor OFF (non déclaré donc couleur par défaut darkgrey
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> luSwitchAttachment; // attachement APVTS
    L_CalibSlider LCalibSlider{colorTitle};
    //juce::Slider LCalibSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> luCalibAttachement; // attachement APVTS

    //==============================================================================


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussMeter2_1AudioProcessorEditor)
};
