
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SC16AudioProcessor::SC16AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    treeState{ *this,nullptr,"PARAMETERS", createParameterLayout() }

#endif
{
    treeState.addParameterListener("GAIN", this);
    treeState.addParameterListener("MIX", this);
    treeState.addParameterListener("ALGORITHM", this);
}

SC16AudioProcessor::~SC16AudioProcessor()
{
  
    treeState.removeParameterListener("GAIN", this);
    treeState.removeParameterListener("MIX", this);
    treeState.removeParameterListener("ALGORITHM", this);
}

//==============================================================================
const juce::String SC16AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SC16AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SC16AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SC16AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SC16AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SC16AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SC16AudioProcessor::getCurrentProgram()
{
    return 0;
}

void SC16AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SC16AudioProcessor::getProgramName (int index)
{
    return {};
}

void SC16AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}



//==============================================================================
void SC16AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();

    irLoader.reset();
    irLoader.prepare(spec);

    mix = juce::jmap(treeState.getRawParameterValue("MIX")->load(), 0.0f, 100.f, 0.0f, 1.0f);
}

void SC16AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SC16AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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



// ****************************************************************************
// **********************PROCESS BLOCK ****************************************
// ****************************************************************************

void SC16AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    juce::dsp::AudioBlock<float>block{ buffer };

    juce::dsp::DryWetMixer<float>wetDryMixer;
    float wetDryValue = *treeState.getRawParameterValue("MIX") / 100.0f;

    wetDryMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    wetDryMixer.setWetMixProportion(wetDryValue);
    wetDryMixer.setWetLatency(0.0);

    wetDryMixer.prepare(spec);


    // the dry buffer
    wetDryMixer.pushDrySamples(buffer);

    // for IR loader
    if (irLoader.getCurrentIRSize() > 0)
    {
        irLoader.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // the wet buffer
    wetDryMixer.mixWetSamples(block);

    // for output gain
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* data = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            auto input = data[sample];
            float output;

            data[sample] = buffer.getSample(channel, sample) * rawVolume;

        }
    }


    // for level meters
    rmsLevelLeft = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
    rmsLevelRight = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));


}

//==============================================================================
bool SC16AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SC16AudioProcessor::createEditor()
{
    return new SC16AudioProcessorEditor (*this);
}

//==============================================================================
void SC16AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SC16AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));

    // Debug command to print XML content
    DBG("XML Content:\n" << xmlState->toString());

    //*************************
    // Extract the value of "IRCHOICE" parameter
    int irChoice = treeState.getRawParameterValue("ALGORITHM")->load();

    // Call the loadIRbinary function based on the extracted value

    // Adjust the ComboBox ID to match ValueTree state index
    int adjustedId = irChoice + 1;


    switch (adjustedId)
    {
    case 1:
        irLoader.reset();
        break;
    case 2:
        loadIRbinary("ambience_wav", BinaryData::ambience_wavSize, BinaryData::ambience_wavSize);
        break;
    case 3:
        loadIRbinary("hall_wav", BinaryData::hall_wavSize, BinaryData::hall_wavSize);
        break;
    case 4:
        loadIRbinary("hall2_wav", BinaryData::hall2_wavSize, BinaryData::hall2_wavSize);
        break;
    case 5:
        loadIRbinary("nonlin_wav", BinaryData::nonlin_wavSize, BinaryData::nonlin_wavSize);
        break;
    case 6:
        loadIRbinary("plate_wav", BinaryData::plate_wavSize, BinaryData::plate_wavSize);
        break;
    case 7:
        loadIRbinary("plate2_wav", BinaryData::plate2_wavSize, BinaryData::plate2_wavSize);
        break;
    case 8:
        loadIRbinary("reversed_wav", BinaryData::reversed_wavSize, BinaryData::reversed_wavSize);
        break;
    case 9:
        loadIRbinary("room_wav", BinaryData::room_wavSize, BinaryData::room_wavSize);
        break;
    default:
        break;
    }

}


juce::AudioProcessorValueTreeState::ParameterLayout SC16AudioProcessor::createParameterLayout()
{


    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto gainParam = std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", -48.0f, 0.0f, -10.0f);
    params.push_back(std::move(gainParam));

    auto mixParam = std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.f, 100.0f, 20.0f);
    params.push_back(std::move(mixParam));

    auto comboParams = std::make_unique<juce::AudioParameterChoice>("ALGORITHM", "Algorithm", juce::StringArray("No Algorithm Loaded!",
        "Ambience",
        "Hall",
        "Hall2",
        "NonLin",
        "Plate",
        "Plate2",
        "Reversed",
        "Room"), 0);
    params.push_back(std::move(comboParams));



    return { params.begin(), params.end() };
}

void SC16AudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{

    mix = juce::jmap(treeState.getRawParameterValue("MIX")->load(), 0.0f, 100.f, 0.0f, 1.0f);


}

float SC16AudioProcessor::getRMSValue(const int channel) const
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0)
        return rmsLevelLeft;
    if (channel == 1)
        return rmsLevelRight;
    return 0.f;
}



// IR Binary loader

void SC16AudioProcessor::loadIRbinary(const char* resourceName, int dataSizeInBytes, size_t resourceSize)
{
    const void* sourceData = BinaryData::getNamedResource(resourceName, dataSizeInBytes);

    irLoader.reset(); // clears the buffer for next ir file
    irLoader.loadImpulseResponse(sourceData, resourceSize, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, 0, juce::dsp::Convolution::Normalise::yes);

}




//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SC16AudioProcessor();
}
