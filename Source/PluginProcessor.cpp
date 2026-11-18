/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONSTRUCTEUR
//==============================================================================
PiroussMeter2_1AudioProcessor::PiroussMeter2_1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
	// --- APVTS & PARAMÈTRES ---
	apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
	PeakOrLuViewParam = apvts.getRawParameterValue("PEAKORLUVIEW");
    showCommandViewParam = apvts.getRawParameterValue("COMMANDVIEW");
    showDisplayViewParam = apvts.getRawParameterValue("DISPLAYVIEW");
    KSwitchParam = apvts.getRawParameterValue("KSWITCHVIEW");
    KSliderParam = apvts.getRawParameterValue("KSLIDERCALIB");
    LuSwitchParam = apvts.getRawParameterValue("LUSWITCHVIEW");
    LuSliderParam = apvts.getRawParameterValue("LUSLIDERCALIB");
}

//==============================================================================
// DESTRUCTEUR
//==============================================================================
PiroussMeter2_1AudioProcessor::~PiroussMeter2_1AudioProcessor()
{
}

//==============================================================================
const juce::String PiroussMeter2_1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PiroussMeter2_1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PiroussMeter2_1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PiroussMeter2_1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PiroussMeter2_1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PiroussMeter2_1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PiroussMeter2_1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PiroussMeter2_1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PiroussMeter2_1AudioProcessor::getProgramName (int index)
{
    return {};
}

void PiroussMeter2_1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// --- PREPARE TO PLAY ---
//==============================================================================
void PiroussMeter2_1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Initialisation des DSP PPM, RMS avec le sample rate de l'hôte
    ppmDetector.prepare(sampleRate);
    rmsDetector.prepare(sampleRate);
    
    // Initialisation du détecteur LU (EBU R128) avec tous les paramètres requis
    const int numChannels = getTotalNumInputChannels();
    luDetector.prepareToPlay(sampleRate, numChannels, samplesPerBlock, 30); // 30 Hz par exemple pour le taux de rafraîchissement

    // Création de la structure de configuration requise par le DSP JUCE (et l'oversampling du True Peak)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumInputChannels());

    // Initialisation du DSP TP avec le spec complet
    tpDetector.prepare(spec);
}

//==============================================================================
// --- RELEASE RESSOURCES ---
//==============================================================================
void PiroussMeter2_1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PiroussMeter2_1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
// --- PROCESS BLOCK ---
//==============================================================================

void PiroussMeter2_1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Nettoyage des canaux de sortie excédentaires
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Savoir si le DAW jouait ou pas...
    if (auto* playHead = getPlayHead())
    {
        auto optionalPos = playHead->getPosition();
        if (optionalPos.hasValue())
        {
            bool isPlaying = optionalPos->getIsPlaying();

            // Détection du front montant : le DAW vient de passer de "Stoppé/Pause" à "Lecture"
            if (isPlaying && !wasPlaying)
            {
                tpDetector.reset();
                luDetector.reset();
            }

            wasPlaying = isPlaying;
        }
    }

    // --- ICI : On envoie le buffer audio dans nos détecteurs (UNE SEULE FOIS pour tout le bloc) ---
    ppmDetector.processBlock(buffer);
    rmsDetector.processBlock(buffer);
    tpDetector.processBlock(buffer);
    luDetector.processBlock(buffer);
    
}

//==============================================================================
bool PiroussMeter2_1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PiroussMeter2_1AudioProcessor::createEditor()
{
    return new PiroussMeter2_1AudioProcessorEditor (*this);
}
//==============================================================================
// --- GET & SET STATE ---
//==============================================================================
void PiroussMeter2_1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto* editor = dynamic_cast<PiroussMeter2_1AudioProcessorEditor*>(getActiveEditor()))
    {
        apvts.state.setProperty("editorWidth", editor->getWidth(), nullptr);
        apvts.state.setProperty("editorHeight", editor->getHeight(), nullptr);
    }
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PiroussMeter2_1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// --- APVTS - PARAMETERS LAYOUT ---
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PiroussMeter2_1AudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout; // Création d'un layout de paramètres

    // --- PARAM --- Peak or LU View
	layout.add(std::make_unique<juce::AudioParameterChoice>(    // Création d'un paramètre de type choix à Layout
        "PEAKORLUVIEW",                     // ID unique
        "Peak Or LU View",                  // Nom
        juce::StringArray{ "Both", "Peak", "LU" }, // Choix
        0                                   // Valeur par défaut (0 = Both)
    ));
    // Masquer / Afficher les zones de commande (par défaut : true)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "COMMANDVIEW", 1 }, "Show Command Zones", true));

    // Masquer / Afficher les zones de display (par défaut : true)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "DISPLAYVIEW", 1 }, "Show Display Zones", true));

    // K-SWITCH ABS ou REL => REL par défaut
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "KSWITCHVIEW", 1 }, "K Switch Display", true));

    // K CALIB SLIDER => K20 (0), K18 (1 - défaut), K14 (2), K12 (3)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "KSLIDERCALIB", 1 },
        "K Calibration",
        juce::StringArray{ "K-20", "K-18", "K-14", "K-12" }, // Les options
        1 // Valeur par défaut : Index 1 (K-18)
    ));

    // Lu-SWITCH ABS ou REL => REL par défaut
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "LUSWITCHVIEW", 1 }, "Lu Switch Display", true));

    // Lu-CALIB SLIDER => -23 (0), -19 (1 - défaut), -15 (2)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "LUSLIDERCALIB", 1 },
        "LU Calibration",
        juce::StringArray{ "-23", "-19", "-15" }, // Les options
        1 // Valeur par défaut : Index 1 (-19)
    ));
    
	return layout;
}

//==============================================================================
// --- APVTS - GETTERS ---
//==============================================================================
bool PiroussMeter2_1AudioProcessor::getBoolParameter(const juce::String& parameterID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(parameterID)))
        return param->get();

    return false; // Sécurité si l'ID est introuvable
}

float PiroussMeter2_1AudioProcessor::getFloatParameter(const juce::String& parameterID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(parameterID)))
        return param->get();

    return 0.0f;
}

int PiroussMeter2_1AudioProcessor::getChoiceParameter(const juce::String& parameterID) const
{
    if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(parameterID)))
        return param->getIndex();

    return 0;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PiroussMeter2_1AudioProcessor();
}
