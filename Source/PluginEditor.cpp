
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SC16AudioProcessorEditor::SC16AudioProcessorEditor (SC16AudioProcessor& p)
    : AudioProcessorEditor (&p), 
    audioProcessor (p),
    verticalDiscreteMeterL{ [&] {return audioProcessor.getRMSValue(0); } },
    verticalDiscreteMeterR{ [&] {return audioProcessor.getRMSValue(1); } }
{
    
    // gain slider
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    gainSlider.setLookAndFeel(&otherLookandFeel);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 200, 25);
    gainSlider.setValue(-10.0f);
    gainSlider.setTextValueSuffix(" dB");
    gainSlider.setRange(-48.0f, 0.0f);
    gainSlider.setNumDecimalPlacesToDisplay(1);
    gainSlider.addListener(this);
    gainSliderAttachment = std::make_unique<SliderAttachment>(audioProcessor.treeState, "GAIN", gainSlider);

    // gain label
    addAndMakeVisible(gainLabel);
    gainLabel.setLookAndFeel(&otherLookandFeel);
    gainLabel.attachToComponent(&gainSlider, false);
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    juce::Font gainfont = gainLabel.getFont();
    gainfont.setHeight(20.0f);
    gainfont.setBold(true);
    gainLabel.setFont(gainfont);
    gainLabel.setJustificationType(juce::Justification::centred);



    // mix slider
    addAndMakeVisible(mixSlider);
    mixSlider.setLookAndFeel(&otherLookandFeel);
    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 200, 25);
    mixSlider.setTextValueSuffix(" %");
    mixSlider.setRange(0.f, 100.f);
    mixSlider.setNumDecimalPlacesToDisplay(0);
    mixSlider.setValue(0.0f);
    mixSlider.addListener(this);
    mixSliderAttachment = std::make_unique<SliderAttachment>(audioProcessor.treeState, "MIX", mixSlider);

    // mix label
    addAndMakeVisible(mixLabel);
    mixLabel.setLookAndFeel(&otherLookandFeel);
    mixLabel.attachToComponent(&mixSlider, false);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    juce::Font mixfont = mixLabel.getFont();
    mixfont.setHeight(20.0f);
    mixfont.setBold(true);
    mixLabel.setFont(mixfont);
    mixLabel.setJustificationType(juce::Justification::centred);

    // ir menu combobox
    addAndMakeVisible(irMenu);
    irMenu.addItem("No Algorithm Loaded!", 1);
    irMenu.addItem("Ambience", 2);
    irMenu.addItem("Hall", 3);
    irMenu.addItem("Hall 2", 4);
    irMenu.addItem("NonLin", 5);
    irMenu.addItem("Plate", 6);
    irMenu.addItem("Plate2", 7);
    irMenu.addItem("Reversed", 8);
    irMenu.addItem("Room", 9);
    irMenu.setSelectedId(0);
    irMenuAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.treeState, "ALGORITHM", irMenu);

    // irLoader label
    addAndMakeVisible(roomLabel);
    roomLabel.setLookAndFeel(&otherLookandFeel);
    roomLabel.attachToComponent(&irMenu, false);
    roomLabel.setText("Algorithm", juce::dontSendNotification);
    roomLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    juce::Font roomlabelfont = roomLabel.getFont();
    roomlabelfont.setHeight(20.0f);
    roomlabelfont.setBold(true);
    roomLabel.setFont(roomlabelfont);
    roomLabel.setJustificationType(juce::Justification::centred);


    // info button
    addAndMakeVisible(infoButton);
    infoButton.setLookAndFeel(&otherLookandFeel);
    infoButton.setButtonText("?");
    infoButton.onClick = [this]()
        {
            infoWindow = std::make_unique<juce::AlertWindow>("Info",
                " Enter message here",
                juce::MessageBoxIconType::InfoIcon,
                nullptr);

            juce::String infoMessage{
                "Impulse responses captured by RJ Baldwin at 96kHz from an original AMS RMX 16 unit at Futureworks Manchester. "
            };
            infoWindow->showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Info", infoMessage, "OK", nullptr, juce::ModalCallbackFunction::create([this](int result)
                {

                }

            ));

        };


    irMenu.onChange = [this]
        {
            int selectedId = irMenu.getSelectedId();


            DBG("Combo Box Selection Changed"); // Add this line for debugging
            DBG("ID: " << selectedId);



            switch (selectedId)
            {
            case 1:
                audioProcessor.irLoader.reset();
                break;
            case 2:
                audioProcessor.loadIRbinary("ambience_wav", BinaryData::ambience_wavSize, BinaryData::ambience_wavSize);
                break;
            case 3:
                audioProcessor.loadIRbinary("hall_wav", BinaryData::hall_wavSize, BinaryData::hall_wavSize);
                break;
            case 4:
                audioProcessor.loadIRbinary("hall2_wav", BinaryData::hall2_wavSize, BinaryData::hall2_wavSize);
                break;
            case 5:
                audioProcessor.loadIRbinary("nonlin_wav", BinaryData::nonlin_wavSize, BinaryData::nonlin_wavSize);
                break;
            case 6:
                audioProcessor.loadIRbinary("plate_wav", BinaryData::plate_wavSize, BinaryData::plate_wavSize);
                break;
            case 7:
                audioProcessor.loadIRbinary("plate2_wav", BinaryData::plate2_wavSize, BinaryData::plate2_wavSize);
                break;
            case 8:
                audioProcessor.loadIRbinary("reversed_wav", BinaryData::reversed_wavSize, BinaryData::reversed_wavSize);
                break;
            case 9:
                audioProcessor.loadIRbinary("room_wav", BinaryData::room_wavSize, BinaryData::room_wavSize);
                break;
            default:
                break;
            }



        };


    // level meter  
    addAndMakeVisible(verticalDiscreteMeterL);
    addAndMakeVisible(verticalDiscreteMeterR);
    startTimerHz(24);

    //==============================================================================


    setSize(410, 410);
}

SC16AudioProcessorEditor::~SC16AudioProcessorEditor()
{
}


void SC16AudioProcessorEditor::timerCallback()
{
    /*   horizontalMeterL.setLevel(audioProcessor.getRMSValue(0));
       horizontalMeterR.setLevel(audioProcessor.getRMSValue(1));
       horizontalMeterL.repaint();
       horizontalMeterR.repaint();*/
}

//==============================================================================
void SC16AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("SCS16 V1 - RJBaldwin", getLocalBounds(), juce::Justification::centredTop, 1);

    //auto background = juce::ImageCache::getFromMemory(BinaryData::guiBack1_jpg, BinaryData::guiBack1_jpgSize);
   // g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::doNotResize);
}

void SC16AudioProcessorEditor::resized()
{
    // combo box
    irMenu.setBounds(145, 300, 120, 25);

    // gain slider
    gainSlider.setBounds(50, 300, 80, 80);

    // mix slider
    mixSlider.setBounds(280, 300, 80, 80);

    // level meter
    verticalDiscreteMeterL.setBounds(180, 80, 25, 200);
    verticalDiscreteMeterR.setBounds(215, 80, 25, 200);

    // info button
    infoButton.setBounds(370, 350, 30, 30);
}

// gain to decibels

void  SC16AudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
    {
        audioProcessor.rawVolume = pow(10, gainSlider.getValue() / 20);
        DBG("Raw Volume: " << audioProcessor.rawVolume);
    }

}