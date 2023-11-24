
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SC16AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SC16AudioProcessorEditor (SC16AudioProcessor&);
    ~SC16AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SC16AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC16AudioProcessorEditor)
};
