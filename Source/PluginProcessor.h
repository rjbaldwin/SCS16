// AMS RMX16 inspired reverb plugin using IRs that i took from an original unit at work! 
// Created by Robert J Baldwin of Supercoiled Samples 24/11/2023
// v1.0

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SC16AudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SC16AudioProcessor();
    ~SC16AudioProcessor() override;

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

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    float getRMSValue(const int channel) const;
    void loadIRbinary(const char* resourceName, int dataSizeInBytes, size_t resourceSize);


    juce::File root, savedFile;
    juce::dsp::Convolution irLoader;
    juce::AudioProcessorValueTreeState treeState;

    // for gain
    double rawVolume{};
    // for mix
    float mix{ 0.0 };

private:

    juce::dsp::ProcessSpec spec;
    float rmsLevelLeft, rmsLevelRight;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC16AudioProcessor)
};
