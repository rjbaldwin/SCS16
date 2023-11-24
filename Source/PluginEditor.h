
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "BinaryData.h"
#include "Component/VerticalDiscreteMeter.h"

//==============================================================================

// the look and feel class
class OtherLookAndFeel : public juce::LookAndFeel_V1
{
public:


};


class SC16AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::Timer
{
public:
    SC16AudioProcessorEditor (SC16AudioProcessor&);
    ~SC16AudioProcessorEditor() override;

    //==============================================================================
    void timerCallback() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SC16AudioProcessor& audioProcessor;

    OtherLookAndFeel otherLookandFeel;

    juce::ComboBox irMenu;
    juce::Label roomLabel;


    juce::Slider gainSlider;
    std::unique_ptr<SliderAttachment> gainSliderAttachment;
    juce::Label gainLabel;

    juce::Slider mixSlider;
    std::unique_ptr<SliderAttachment> mixSliderAttachment;
    juce::Label mixLabel;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboBoxAttachment> irMenuAttachment;

    // for metering
    //Gui::HorizontalMeter horizontalMeterL, horizontalMeterR;
    Gui::VerticalDiscreteMeter verticalDiscreteMeterL, verticalDiscreteMeterR;

    // info window
    juce::TextButton infoButton;
    std::unique_ptr<juce::AlertWindow> infoWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SC16AudioProcessorEditor)
};
