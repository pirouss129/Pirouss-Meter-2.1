/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/PPMDsp.h"
#include "DSP/TPDsp.h"
#include "DSP/RMSDsp.h"
#include "DSP/libebur128/EbuR128Wrapper.h"



//==============================================================================
/**
*/
class PiroussMeter2_1AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PiroussMeter2_1AudioProcessor();
    ~PiroussMeter2_1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	// Custom methods for the plugin

    // --- APVTS & PARAMÈTRES ---
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    // --- APVTS Getters ---
    bool getBoolParameter(const juce::String& parameterID) const;
    float getFloatParameter(const juce::String& parameterID) const;
    int getChoiceParameter(const juce::String& parameterID) const;

    PpmDetector ppmDetector;    // construction objet PPM
    TPDsp tpDetector;   // construction objet True Peak
    RMSDsp rmsDetector;     // construction de l'objet RMS
    Ebu128LoudnessMeter luDetector; // construction de l'objet LU


private:

    // --- Cache de pointeurs atomiques pour la boucle audio (Performances Thread Audio) ---
	std::atomic<float>* PeakOrLuViewParam = nullptr;
    std::atomic<float>* showCommandViewParam = nullptr;
    std::atomic<float>* showDisplayViewParam = nullptr;
    std::atomic<float>* KSwitchParam = nullptr;
    std::atomic<float>* KSliderParam = nullptr;
    std::atomic<float>* LuSwitchParam = nullptr;
    std::atomic<float>* LuSliderParam = nullptr;
    
    // --- Variables privées ---
    bool wasPlaying = false;    // sert à savoir si le DAW jouait ou pas pour réinitialiser la mesure

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PiroussMeter2_1AudioProcessor)
};
