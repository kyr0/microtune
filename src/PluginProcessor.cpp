/***************************************************************
 ** Copyright (C) 2021 Aron Homberg
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** AUDIOAPP IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "Constants.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

AppAudioProcessor::AppAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
   MagicProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true)),
#endif
   parameters (*this, nullptr, juce::Identifier ("APVTSTutorial"),
   {
           std::make_unique<juce::AudioParameterFloat> ("gain",            // parameterID
                                                        "Gain",            // parameter name
                                                        0.0f,              // minimum value
                                                        1.0f,              // maximum value
                                                        0.5f),             // default value
           std::make_unique<juce::AudioParameterBool> ("invertPhase",      // parameterID
                                                       "Invert Phase",     // parameter name
                                                       false)              // default value
   })
{
    FOLEYS_SET_SOURCE_PATH (__FILE__);

    phaseParameter = parameters.getRawParameterValue ("invertPhase");
    gainParameter  = parameters.getRawParameterValue ("gain");
}

AppAudioProcessor::~AppAudioProcessor()
{
}

double AppAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AppAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AppAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AppAudioProcessor::setCurrentProgram (int )
{
}

const String AppAudioProcessor::getProgramName (int )
{
    return String();
}

void AppAudioProcessor::changeProgramName (int , const String& )
{
}

void AppAudioProcessor::prepareToPlay (double , int )
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need
    auto phase = *phaseParameter < 0.5f ? 1.0f : -1.0f;
    previousGain = *gainParameter * phase;
}

void AppAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AppAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
 #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void AppAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
   MidiBuffer processedMidi; // buffer that holds output midi (initially empty)

   double samplerate = getSampleRate();

   // clear all buffers. We don't produce audio nor do we filter incoming audio.
   buffer.clear();
   
   // flip from incoming midi data to outgoing data
   midiMessages.swapWith(processedMidi);
}


juce::ValueTree AppAudioProcessor::createGuiValueTree()
{
    juce::String text (BinaryData::magic_xml, BinaryData::magic_xmlSize);
    return juce::ValueTree::fromXml (text);
}


AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
   return new AppAudioProcessor();
}
