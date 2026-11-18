/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PiroussMeter2_1AudioProcessorEditor::PiroussMeter2_1AudioProcessorEditor (PiroussMeter2_1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
	// add components
	addAndMakeVisible(titlePlate);

	addAndMakeVisible(rm3LBargraph);
		rm3LBargraph.setScalePosition(RM3Bargraph::ScalePosition::Left);
	addAndMakeVisible(rtw40Bargraph);
	addAndMakeVisible(rm3RBargraph);
		rm3RBargraph.setScalePosition(RM3Bargraph::ScalePosition::Right);
	addAndMakeVisible(tpDisplay);
	addAndMakeVisible(KCommandPanel);
	addAndMakeVisible(KSwitch);
		kSwitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts,"KSWITCHVIEW",KSwitch.myButton);
	addAndMakeVisible(KCalibSlider);
		kCalibAttachement = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts,
		"KSLIDERCALIB",
		KCalibSlider.slider 
	);

	addAndMakeVisible(luBargraph);
	addAndMakeVisible(integratedDisplay);
	addAndMakeVisible(lraDisplay);
	addAndMakeVisible(LCommandPanel);
	addAndMakeVisible(LSwitch);
		luSwitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "LUSWITCHVIEW", LSwitch.myButton);
	addAndMakeVisible(LCalibSlider);
		luCalibAttachement = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.apvts,
		"LUSLIDERCALIB",
		LCalibSlider.slider 
			
	);

	// INTERCEPTION CLICS
	titlePlate.setInterceptsMouseClicks(false, false); // le composant laisse passer les clicks, les enfants contenus dans le composant laisse passer les clicks
	
	rm3LBargraph.setInterceptsMouseClicks(false, false);
	rtw40Bargraph.setInterceptsMouseClicks(false, false);
	rm3RBargraph.setInterceptsMouseClicks(false, false);
	tpDisplay.setInterceptsMouseClicks(false, false);
		
	luBargraph.setInterceptsMouseClicks(false, false);
	integratedDisplay.setInterceptsMouseClicks(false, false);
	lraDisplay.setInterceptsMouseClicks(false, false);


	// INITIALISATION AFFICHAGE PEAK OU LU OU LES DEUX à partir de l'état sauvegardé dans l'APVTS
	int choiceIndex = audioProcessor.getChoiceParameter("PEAKORLUVIEW");	// création variable choiceIndex = valeur récupérée avec la méthode getChoiceParameter
	currentPeakOrLuViewMode = static_cast<peakOrLuViewMode>(choiceIndex);	//conversion de float stocké dans l'APVTS en peakOrLuViewMode (array de 0 à 2)
	prevPeakOrLuViewMode = currentPeakOrLuViewMode;	// l'ancienne valeur prend la valeur actuelle (utile pour comparaison)

	
	// INITIALISATION COMMAND VIEW à partir de l'état sauvegardé dans l'APVTS
	currentCommandViewState = audioProcessor.getBoolParameter("COMMANDVIEW");
	prevCommandViewState = currentCommandViewState;

	// INITIALISATION DISPLAY VIEW à partir de l'état sauvegardé dans l'APVTS
	currentDisplayViewState = audioProcessor.getBoolParameter("DISPLAYVIEW");
	prevDisplayViewState = currentDisplayViewState;

	// RESIZE
	// Récupération des modes d'affichages et calcul des limites
	int initialMinWidth = minWidth;
	if (currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly)
		initialMinWidth = static_cast<int>(minWidth * 0.6f);
	else if (currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly)
		initialMinWidth = static_cast<int>(minWidth * 0.4f);

	int initialMinHeight = minHeight;
	if (!currentCommandViewState)
		initialMinHeight -= commandHeight;
	if (!currentDisplayViewState)
		initialMinHeight -= displayHeight;

	// Restauration de la taille et de la position du plugin à partir des préférences
	const int savedW = audioProcessor.apvts.state.getProperty("editorWidth", defaultWidth);
	const int savedH = audioProcessor.apvts.state.getProperty("editorHeight", defaultHeight);
    setSize (savedW, savedH);
	setResizeLimits(initialMinWidth, initialMinHeight, maxWidth, maxHeight);
	setResizable(true, true);

	// --- ABONNEMENTS APVTS LISTENERS ---
	audioProcessor.apvts.addParameterListener("PEAKORLUVIEW", this);
	audioProcessor.apvts.addParameterListener("COMMANDVIEW", this);
	audioProcessor.apvts.addParameterListener("DISPLAYVIEW", this);
	audioProcessor.apvts.addParameterListener("KSWITCHVIEW", this);
	audioProcessor.apvts.addParameterListener("KSLIDERCALIB", this);
	audioProcessor.apvts.addParameterListener("LUSWITCHVIEW", this);
	audioProcessor.apvts.addParameterListener("LUSLIDERCALIB", this);

	// --- SYNCHRO INITIALE AU DÉMARRAGE ---
	rm3LBargraph.setKSystemModeByIndex(audioProcessor.getChoiceParameter("KSLIDERCALIB"));
	rm3RBargraph.setKSystemModeByIndex(audioProcessor.getChoiceParameter("KSLIDERCALIB"));

	luBargraph.setTargetLUFSByIndex(audioProcessor.getChoiceParameter("LUSLIDERCALIB"));

	rm3LBargraph.setDisplayMode(!audioProcessor.getBoolParameter("KSWITCHVIEW"));
	rm3RBargraph.setDisplayMode(!audioProcessor.getBoolParameter("KSWITCHVIEW"));
	luBargraph.setDisplayMode(!audioProcessor.getBoolParameter("LUSWITCHVIEW")); // ou LUSWITCHVIEW selon ton param


	startTimerHz(30); // 3. Lancer le timer à 30 images/sec
	
}

PiroussMeter2_1AudioProcessorEditor::~PiroussMeter2_1AudioProcessorEditor()
{
	audioProcessor.apvts.removeParameterListener("PEAKORLUVIEW", this);
	audioProcessor.apvts.removeParameterListener("COMMANDVIEW", this);
	audioProcessor.apvts.removeParameterListener("DISPLAYVIEW", this);
	audioProcessor.apvts.removeParameterListener("KSWITCHVIEW", this);
	audioProcessor.apvts.removeParameterListener("KSLIDERCALIB", this);
	audioProcessor.apvts.removeParameterListener("LUSWITCHVIEW", this);
	audioProcessor.apvts.removeParameterListener("LUSLIDERCALIB", this);
}

//==============================================================================
// --- PARAMETER CHANGED --- METHODE PERSO ---
//==============================================================================

void PiroussMeter2_1AudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
	if (parameterID == "PEAKORLUVIEW")
	{
		int choiceIndex = audioProcessor.getChoiceParameter("PEAKORLUVIEW");
		currentPeakOrLuViewMode = static_cast<peakOrLuViewMode>(choiceIndex);
		// Appelle ici ta méthode de mise en page / resize si besoin
	}
	else if (parameterID == "COMMANDVIEW")
	{
		currentCommandViewState = audioProcessor.getBoolParameter("COMMANDVIEW");
		// Mets à jour l'affichage de ton panneau de commande
	}
	else if (parameterID == "DISPLAYVIEW")
	{
		currentDisplayViewState = audioProcessor.getBoolParameter("DISPLAYVIEW");
		// Mets à jour l'affichage de tes displays
	}
	else if (parameterID == "KSWITCHVIEW")
	{
		bool isRelativ = audioProcessor.getBoolParameter("KSWITCHVIEW");
		rm3LBargraph.setDisplayMode(!isRelativ);
		rm3RBargraph.setDisplayMode(!isRelativ);
	}
	else if (parameterID == "KSLIDERCALIB")
	{
		int currentKIndex = audioProcessor.getChoiceParameter("KSLIDERCALIB"); // Renvoie 0, 1, 2 ou 3

		// Transmettre la calibration choisie au bargraph (K-20, K-18, K-14 ou K-12)
		rm3LBargraph.setKSystemModeByIndex(currentKIndex);
		rm3RBargraph.setKSystemModeByIndex(currentKIndex);
	}
	else if (parameterID == "LUSWITCHVIEW")
	{
		bool isRelativ = audioProcessor.getBoolParameter("LUSWITCHVIEW"); // Récupère le true/false actuel
		// On passe l'info aux bargraphs
		luBargraph.setDisplayMode(!isRelativ);
	}
	else if (parameterID == "LUSLIDERCALIB")
	{
		int currentLuIndex = audioProcessor.getChoiceParameter("LUSLIDERCALIB"); // Renvoie 0, 1 ou 2
		// Transmettre la calibration choisie au bargraph (-23, -19 ou -15)
		luBargraph.setTargetLUFSByIndex(currentLuIndex);
	}
}

//==============================================================================
// --- TIMER CALLBACK ---
//==============================================================================

void PiroussMeter2_1AudioProcessorEditor::timerCallback()
{
	// ==========================================
	// 1. RÉCUPÉRATION DES VALEURS AUDIO & AFFICHAGE (Le vrai rôle du 30 fps)
	// ==========================================

	// Récupération des niveaux PPM depuis le processeur (via le PpmDetector)
	float qppmL = audioProcessor.ppmDetector.getQppmLeft();
	float qppmR = audioProcessor.ppmDetector.getQppmRight();
	float ppmHoldL = audioProcessor.ppmDetector.getPpmLeft();
	float ppmHoldR = audioProcessor.ppmDetector.getPpmRight();

	// Transmission des niveaux au bargraph RTW
	rtw40Bargraph.setLevels(qppmL, qppmR, ppmHoldL, ppmHoldR);


	// Récupération des niveaux RMS depuis le processeur (via l'objet rmsDetector)
	float rmsShortL = audioProcessor.rmsDetector.getRmsSL();
	float rmsShortR = audioProcessor.rmsDetector.getRmsSR();
	float rmsMomentaryL = audioProcessor.rmsDetector.getRmsML();
	float rmsMomentaryR = audioProcessor.rmsDetector.getRmsMR();

	// Transmission des niveaux au bargraph RM3
	rm3LBargraph.setLevels(rmsShortL, rmsMomentaryL);
	rm3RBargraph.setLevels(rmsShortR, rmsMomentaryL);


	// Récupération des true peaks
	float tp = audioProcessor.tpDetector.getTruePeak();

	// Transmission des tp au digital Display
	tpDisplay.setValue(tp);


	// Récupération des valeurs LU
	float luMomentary = audioProcessor.luDetector.getMomentaryLoudness();
	float luShort = audioProcessor.luDetector.getShortTermLoudness();
	float lra = audioProcessor.luDetector.getLoudnessRange();
	float integrated = audioProcessor.luDetector.getIntegratedLoudness();

	// Transmission des valeurs LU au bargraph
	luBargraph.setLevels(luMomentary, luShort, lra, integrated);
	lraDisplay.setValue(lra);


	// --- GESTION DU MODE ABSOLU / RELATIF POUR L'INTEGRATED DISPLAY ---
	float displayIntegrated = integrated; // Par défaut en valeur absolue

	// On utilise notre getter propre directement (beaucoup plus léger qu'un dynamic_cast en boucle !)
	bool isRelativ = audioProcessor.getBoolParameter("LUSWITCHVIEW");

	if (isRelativ)
	{
		int currentLuIndex = audioProcessor.getChoiceParameter("LUSLIDERCALIB");

		float targetLevel = -23.0f;
		if (currentLuIndex == 1) targetLevel = -19.0f;
		else if (currentLuIndex == 2) targetLevel = -15.0f;

		displayIntegrated = integrated - targetLevel;
	}

	// Transmission de la valeur calculée au display
	integratedDisplay.setValue(displayIntegrated);
}

//==============================================================================
// --- PAINT ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(colorBackground);   // clear the background
}

//==============================================================================
// --- RESIZED ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::resized()
{
	auto pluginBounds = getLocalBounds();

	// header
	headerArea = pluginBounds.removeFromTop(headerHeight);
		titlePlate.setBounds(headerArea);


	// --- calcul en fonction de si on affiche peak, Lu, ou les deux
	int totalWidth = pluginBounds.getWidth();
	if(currentPeakOrLuViewMode == peakOrLuViewMode::Both)
	{
		int peakRMSWidth = totalWidth * 0.6f; // largeur de la zone Peak & RMS (60% de la largeur totale)
		peakRMSArea = pluginBounds.removeFromLeft(static_cast<int>(peakRMSWidth)); // la zone prend 60% de la largeur totale
		luArea = pluginBounds; // le reste de la zone (40%) est pour LU & Integrated & LRA & LCommand
	}
	else if(currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly)
	{
		peakRMSArea = pluginBounds;
	}
	else if (currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly)
	{
		luArea = pluginBounds;
	}
	// ---	
	
	

	// Zone de gauche : Peak & RMS & TPDisplay & KCommand
	if (currentCommandViewState)	// si on affiche la zone command
	{
		KCommandArea = peakRMSArea.removeFromBottom(commandHeight); // la zone KCommand prend commandHeight de hauteur
		KCommandPanel.setBounds(KCommandArea);
		KSwitchArea = KCommandArea.removeFromLeft(KCommandArea.getWidth() * 0.4f); // la zone KButton prend la moitié de la largeur de KCommand
		KCalibSliderArea = KCommandArea; // la zone KCalibSlider prend le reste de la zone (la moitié de la largeur de KCommand)
		KSwitch.setBounds(KSwitchArea.reduced(10.0f));
		KCalibSlider.setBounds(KCalibSliderArea.reduced(5.0f));
	}
	
	if (currentDisplayViewState)	// si on affiche la zone Display
	{
		tpDisplayArea = peakRMSArea.removeFromBottom(displayHeight); // la zone TPDisplay prend displayHeight de hauteur
		tpDisplay.setBounds(tpDisplayArea);
	}
	
	
	peakRMSVuArea = peakRMSArea; // la zone Peak & RMS prend le reste de la zone (la hauteur restante après TPDisplay et KCommand)
	int quarterVU = peakRMSVuArea.getWidth() * 0.25f; // largeur d'un VU (1/4 de la largeur de la zone Peak & RMS)
	RM3LArea = peakRMSVuArea.removeFromLeft(quarterVU); // la zone RM3L prend 1/4 de la largeur de la zone Peak & RMS
	RTW40Area = peakRMSVuArea.removeFromLeft(quarterVU*2); // la zone RTW40 prend 1/4 de la largeur de la zone Peak & RMS
	RM3RArea = peakRMSVuArea; // la zone RM3R prend le reste de la zone (1/4 de la largeur de la zone Peak & RMS)
		rm3LBargraph.setBounds(RM3LArea);
		rtw40Bargraph.setBounds(RTW40Area);
		rm3RBargraph.setBounds(RM3RArea);

	// Zone de droite : LU & Integrated & LRA & LCommand
		if (currentCommandViewState)	// si on affiche la zone Command
		{
			LCommandArea = luArea.removeFromBottom(commandHeight); // la zone LCommand prend commandHeight de hauteur
			LCommandPanel.setBounds(LCommandArea);
			LSwitchArea = LCommandArea.removeFromLeft(LCommandArea.getWidth() * 0.5f); // la zone LButton prend la moitié de la largeur de LCommand
			LCalibSliderArea = LCommandArea; // la zone LCalibSlider prend le reste de la zone (la moitié de la largeur de LCommand)
			LSwitch.setBounds(LSwitchArea.reduced(10.0f));
			LCalibSlider.setBounds(LCalibSliderArea.reduced(5.0f));
		}
	
		if (currentDisplayViewState)	// si on affiche la zone Display
		{
			luDisplayArea = luArea.removeFromBottom(displayHeight); // la zone LU Display prend displayHeight de hauteur
			integratedDisplayArea = luDisplayArea.removeFromLeft(luDisplayArea.getWidth() * 0.5f); // la zone Integrated Display prend la moitié de la largeur de luDisplayArea
			lraDisplayArea = luDisplayArea; // la zone LRA Display prend le reste de la zone (la moitié de la largeur de luDisplayArea)
			integratedDisplay.setBounds(integratedDisplayArea);
			lraDisplay.setBounds(lraDisplayArea);
		}
	

	luVuArea = luArea; // la zone LU VU prend le reste de la zone (la hauteur restante après Integrated Display et LRA Display)
		luBargraph.setBounds(luVuArea);

}

//==============================================================================
// --- MENU CLIC DROIT ---
//==============================================================================

void PiroussMeter2_1AudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
	if (event.mods.isPopupMenu()) // si clic droit
	{
		juce::PopupMenu menu; // Préparation du Menu

		menu.addSectionHeader("Affichage");
		menu.addItem(1, "Peak/RMS & LUFS", true, currentPeakOrLuViewMode == peakOrLuViewMode::Both);
		menu.addItem(2, "Peak/RMS", true, currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly);
		menu.addItem(3, "LU", true, currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly);
		menu.addSeparator();
		menu.addItem(4, "Show Displays", true, currentDisplayViewState);
		menu.addItem(5, "Show Command", true, currentCommandViewState);
		menu.addSeparator();
		
#if JucePlugin_Build_Standalone
		// On récupère la fenêtre pour cocher l'item si elle est déjà on top
		bool isCurrentlyOnTop = false;
		if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
			isCurrentlyOnTop = window->isAlwaysOnTop();

		menu.addItem(6, "Always on top", true, isCurrentlyOnTop);
#endif
		menu.addSeparator();
		menu.addSectionHeader("Infos");
		menu.addItem(7, "Made with JUCE", true);
		menu.addItem(8, "PPM 0ms attaque, 1.5s release pour -10dB, 500ms hold", false);
		menu.addItem(9, "QPPM 5ms attaque, 0.7s release pour - 10dB, 0ms hold", false);
		menu.addItem(10, "RMS 150ms / 600 ms", false);
		menu.addItem(11, "double clic sur Digital display pour relancer le calcul", false);
		

		menu.showMenuAsync(juce::PopupMenu::Options(), [this](int result) // Menu
			{
				if (result == 1)
				{
					setPeakOrLuViewMode(peakOrLuViewMode::Both);
				}
				if (result == 2)
				{
					setPeakOrLuViewMode(peakOrLuViewMode::PeakOnly);
				}
				if (result == 3)
				{
					setPeakOrLuViewMode(peakOrLuViewMode::LuOnly);
				}
				if (result == 4)
				{
					setDisplayViewState(!currentDisplayViewState); // au clic, j'inverse l'état de la variable
				}
				if (result == 5)
				{
					setCommandViewState(!currentCommandViewState); // au clic, j'inverse l'état de la variable
				}
				

#if JucePlugin_Build_Standalone
				if (result == 6)
				{
					if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
					{
						bool currentState = window->isAlwaysOnTop();
						window->setAlwaysOnTop(!currentState);
					}
				}
#endif // JucePlugin_Build_Standalone

				if (result == 7)
				{
					juce::URL("https://juce.com/").launchInDefaultBrowser();
				}

			});
	}
}

//==============================================================================
// --- DOUBLE CLIC ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
	auto clickPos = event.getPosition();

	if (titlePlate.getBounds().contains(clickPos))
	{
		setToPiroussConfig();
	}
	if (rm3LBargraph.getBounds().contains(clickPos) || rtw40Bargraph.getBounds().contains(clickPos) || rm3RBargraph.getBounds().contains(clickPos) || luBargraph.getBounds().contains(clickPos))
	{
		resetConfig();
	}
	if (tpDisplay.getBounds().contains(clickPos))
	{
		audioProcessor.tpDetector.resetMax();
	}
	if (integratedDisplay.getBounds().contains(clickPos) || lraDisplay.getBounds().contains(clickPos))
	{
		audioProcessor.luDetector.reset();
	}
}


//==============================================================================
// --- METHODES PERSONNALISEES ---
//==============================================================================



//==============================================================================
// --- SET PEAK & RMS OR LU VIEW MODE ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::setPeakOrLuViewMode(peakOrLuViewMode newMode) // type - variable
{
	if (currentPeakOrLuViewMode != newMode) // Vérifie si le mode d'affichage a changé
	{
		currentPeakOrLuViewMode = newMode;
		
		// 1. On synchronise l'APVTS (pour l'automation et la sauvegarde du projet)
		if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("PEAKORLUVIEW")))
		{
			param->beginChangeGesture();
			param->setValueNotifyingHost(param->convertTo0to1(static_cast<int>(newMode)));
			param->endChangeGesture();
		}

		// 2. On met à jour les booléens pour les composants graphiques
		const bool showPeak = (currentPeakOrLuViewMode == peakOrLuViewMode::Both || currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly);
		const bool showLu = (currentPeakOrLuViewMode == peakOrLuViewMode::Both || currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly);
		
		setElementsVisibility();	// méthode pour mettre à jour la visibilité des éléments
		updatePluginSize();			// méthode pour mettre à jour la taille du plug
	}
}

//==============================================================================
// --- SET COMMAND VIEW STATE ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::setCommandViewState(bool newMode)
{
	if (currentCommandViewState != newMode)
	{
		currentCommandViewState = newMode;

		// On stocke dans l'APVTS
		if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("COMMANDVIEW")))
		{
			param->beginChangeGesture();
			param->setValueNotifyingHost(newMode ? 1.0f : 0.0f);
			param->endChangeGesture();
		}

		const bool showCommand = currentCommandViewState;
		
		setElementsVisibility();	// méthode pour mettre à jour la visibilité des éléments
		updatePluginSize();			// méthode pour mettre à jour la taille du plug
	}
}

//==============================================================================
// --- SET DISPLAY VIEW STATE ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::setDisplayViewState(bool newMode)
{
	if (currentDisplayViewState != newMode)
	{
		currentDisplayViewState = newMode;

		// On stocke dans l'APVTS
		if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("DISPLAYVIEW")))
		{
			param->beginChangeGesture();
			param->setValueNotifyingHost(newMode ? 1.0f : 0.0f);
			param->endChangeGesture();
		}

		const bool showDisplay = currentDisplayViewState;

		setElementsVisibility();	// méthode pour mettre à jour la visibilité des éléments
		updatePluginSize();			// méthode pour mettre à jour la taille du plug
	}
}


//==============================================================================
// --- SET TO PIROUSS CONFIG ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::setToPiroussConfig()
{
	// 1. Synchronisation APVTS
	if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("PEAKORLUVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(param->convertTo0to1(static_cast<int>(peakOrLuViewMode::PeakOnly)));
		param->endChangeGesture();
	}

	if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("DISPLAYVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(0.0f); // false
		param->endChangeGesture();
	}

	if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("COMMANDVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(1.0f); // true
		param->endChangeGesture();
	}

	// 2. Mise à jour des états locaux et de la visibilité
	currentPeakOrLuViewMode = peakOrLuViewMode::PeakOnly;
	currentDisplayViewState = false;
	currentCommandViewState = true;

	setElementsVisibility();

	// 3. Calcul des nouvelles limites minimales pour cette configuration compacte
	// En PeakOnly, la largeur min est de 60%.
	// Côté hauteur, les displays sont masqués, donc on retire 'displayHeight'. 
	// (Et comme les commandes sont affichées, on garde 'minHeight' de base, ou on ajuste selon l'état des commandes).
	int targetMinWidth = static_cast<int>(minWidth * 0.6f);
	int targetMinHeight = minHeight - displayHeight; // On enlève la hauteur du display masqué

	// 4. On met à jour les prev pour que updatePluginSize() ou les futurs resized ne soient pas perdus
	prevPeakOrLuViewMode = currentPeakOrLuViewMode;
	prevDisplayViewState = currentDisplayViewState;
	prevCommandViewState = currentCommandViewState;

	// 5. On applique les limites strictes et on force la taille minimale
	setResizeLimits(targetMinWidth, targetMinHeight, maxWidth, maxHeight);
	setSize(targetMinWidth, targetMinHeight);
}

//==============================================================================
// --- RESET CONFIG ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::resetConfig()
{
	// 1. Synchronisation APVTS
	if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.apvts.getParameter("PEAKORLUVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(param->convertTo0to1(static_cast<int>(peakOrLuViewMode::Both)));
		param->endChangeGesture();
	}

	if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("DISPLAYVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(1.0f); // true
		param->endChangeGesture();
	}

	if (auto* param = dynamic_cast<juce::AudioParameterBool*>(audioProcessor.apvts.getParameter("COMMANDVIEW")))
	{
		param->beginChangeGesture();
		param->setValueNotifyingHost(1.0f); // true
		param->endChangeGesture();
	}

	// 2. Mise à jour des états locaux et de la visibilité
	currentPeakOrLuViewMode = peakOrLuViewMode::Both;
	currentDisplayViewState = true;
	currentCommandViewState = true;

	setElementsVisibility();

	
	// 4. On met à jour les prev pour que updatePluginSize() ou les futurs resized ne soient pas perdus
	prevPeakOrLuViewMode = currentPeakOrLuViewMode;
	prevDisplayViewState = currentDisplayViewState;
	prevCommandViewState = currentCommandViewState;

	// 5. On applique les limites strictes et on force la taille minimale
	setSize(defaultWidth, defaultHeight);
	setResizeLimits(minWidth, minHeight, maxWidth, maxHeight);
	
}

//==============================================================================
// --- SET ELEMENTS VISIBILITY ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::setElementsVisibility()
{
	// booléens pour savoir si on affiche peak & RMS ou Lu ou les deux
	const bool showPeak = (currentPeakOrLuViewMode == peakOrLuViewMode::Both || currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly);
	const bool showLu = (currentPeakOrLuViewMode == peakOrLuViewMode::Both || currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly);

	// booléens pour savoir si on affiche les displays (TP, integrated, LRA) en fonction de currentPeakOrLuViewMode et booléen currentDisplayViewState
	const bool showTP = (showPeak && currentDisplayViewState);
	const bool showIntegratedAndLRA = (showLu && currentDisplayViewState);

	// booléens pour savoir si on affiche les commands en fonction de currentPeakOrLuViewMode et booléen currentCommandViewState
	const bool showKCommand = (showPeak && currentCommandViewState);
	const bool showLCommand = (showLu && currentCommandViewState);

	// application des booléens sur la visibilité des éléments
	// Zone gauche Peak & RMS
	rm3LBargraph.setVisible(showPeak);
	rtw40Bargraph.setVisible(showPeak);
	rm3RBargraph.setVisible(showPeak);

	tpDisplay.setVisible(showTP);

	KCommandPanel.setVisible(showKCommand);
	KSwitch.setVisible(showKCommand);
	KCalibSlider.setVisible(showKCommand);

	//zone droite LU
	luBargraph.setVisible(showLu);

	integratedDisplay.setVisible(showIntegratedAndLRA);
	lraDisplay.setVisible(showIntegratedAndLRA);

	LCommandPanel.setVisible(showLCommand);
	LSwitch.setVisible(showLCommand);
	LCalibSlider.setVisible(showLCommand);
	
}


//==============================================================================
// --- UPDATE PLUGIN SIZE ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::updatePluginSize()
{
	int currentWidth = getWidth();
	int currentHeight = getHeight();
	int targetWidth = currentWidth;
	int targetHeight = currentHeight;

	// IMPORTANT : Initialiser les limites minimales avec les valeurs actuelles par défaut
	int newMinWidth = minWidth;
	int newMinHeight = minHeight;

	// Si on change de mode d'affichage horizontal (Peak / LU / Both)
	if (prevPeakOrLuViewMode != currentPeakOrLuViewMode)
	{
		if (prevPeakOrLuViewMode == peakOrLuViewMode::Both)
		{
			if (currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly)
			{
				targetWidth = static_cast<int>(currentWidth * 0.6f);
				newMinWidth = minWidth * 0.6;
			}
			else if (currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly)
			{
				targetWidth = static_cast<int>(currentWidth * 0.4f);
				newMinWidth = minWidth * 0.4;
			}
		}
		else if (prevPeakOrLuViewMode == peakOrLuViewMode::PeakOnly)
		{
			if (currentPeakOrLuViewMode == peakOrLuViewMode::Both)
			{
				targetWidth = static_cast<int>(currentWidth / 0.6f);
				newMinWidth = minWidth;
			}
			else if (currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly)
			{
				targetWidth = static_cast<int>((currentWidth / 0.6f) * 0.4f);
				newMinWidth = minWidth * 0.4;
			}
		}
		else if (prevPeakOrLuViewMode == peakOrLuViewMode::LuOnly)
		{
			if (currentPeakOrLuViewMode == peakOrLuViewMode::Both)
			{
				targetWidth = static_cast<int>(currentWidth / 0.4f);
				newMinWidth = minWidth;
			}
			else if (currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly)
			{
				targetWidth = static_cast<int>((currentWidth / 0.4f) * 0.6f);
				newMinWidth = minWidth * 0.6;
			}
		}
	}
	else
	{
		// Si la largeur ne change pas, on garde la largeur min actuelle adaptée
		if (currentPeakOrLuViewMode == peakOrLuViewMode::PeakOnly) newMinWidth = minWidth * 0.6;
		else if (currentPeakOrLuViewMode == peakOrLuViewMode::LuOnly) newMinWidth = minWidth * 0.4;
		else newMinWidth = minWidth;
	}

	// Si on change l'état des commandes (Afficher / Masquer)
	if (prevCommandViewState != currentCommandViewState)
	{
		if (prevCommandViewState == true && currentCommandViewState == false)
		{
			targetHeight = static_cast<int>(currentHeight - commandHeight);
			newMinHeight = minHeight - commandHeight;
		}
		else if (prevCommandViewState == false && currentCommandViewState == true)
		{
			targetHeight = static_cast<int>(currentHeight + commandHeight);
			newMinHeight = minHeight + commandHeight;
		}
	}
	else
	{
		// On s'assure que minHeight reflète l'état actuel si on ne fait pas de transition directe
		newMinHeight = currentCommandViewState ? minHeight : (minHeight - commandHeight);
	}

	// Si on change l'état des Displays TP INTEGRATED LRA (Afficher / Masquer)
	if (prevDisplayViewState != currentDisplayViewState)
	{
		if (prevDisplayViewState == true && currentDisplayViewState == false)
		{
			targetHeight = static_cast<int>(currentHeight - displayHeight);
			newMinHeight = minHeight - displayHeight;
		}
		else if (prevDisplayViewState == false && currentDisplayViewState == true)
		{
			targetHeight = static_cast<int>(currentHeight + displayHeight);
			newMinHeight = minHeight + displayHeight;
		}
	}
	else
	{
		// On s'assure que minHeight reflète l'état actuel si on ne fait pas de transition directe
		newMinHeight = currentDisplayViewState ? minHeight : (minHeight - displayHeight);
	}

	// On met à jour l'ancien mode pour le prochain coup AVANT de redimensionner
	prevPeakOrLuViewMode = currentPeakOrLuViewMode;
	prevCommandViewState = currentCommandViewState;
	prevDisplayViewState = currentDisplayViewState;

	// On applique la nouvelle taille si la largeur OU la hauteur a changé
	if (targetWidth != currentWidth || targetHeight != currentHeight)
	{
		setResizeLimits(newMinWidth, newMinHeight, maxWidth, maxHeight);
		setSize(targetWidth, targetHeight);	// le resized est fait d'office car Juce a détecté le changement de taille
	}
}


//==============================================================================
// --- DEBUG PAINT OVER CHILDREN AFFICHAGE taille du plug ---
//==============================================================================
void PiroussMeter2_1AudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
	bool isDebugMode = false; // Mettre à true pour activer le debug, false pour désactiver

	if (!isDebugMode)
		return;

	// 1. Récupération des dimensions actuelles
	const int w = getWidth();
	const int h = getHeight();
	const juce::String debugText = juce::String(w) + " x " + juce::String(h) + " px";

	// 2. Définition du petit badge de debug (ex: 110x24 px) en haut à droite
	auto debugBounds = juce::Rectangle<int>(w - 115, 8, 110, 24);

	// 3. Dessin du fond et de la bordure
	g.setColour(juce::Colours::black.withAlpha(0.85f));
	g.fillRoundedRectangle(debugBounds.toFloat(), 4.0f);

	g.setColour(juce::Colours::yellow);
	g.drawRoundedRectangle(debugBounds.toFloat(), 4.0f, 1.0f);

	// 4. Dessin du texte au centre du rectangle
	g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
	g.drawText(debugText, debugBounds, juce::Justification::centred, false);
}
