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

AppAudioProcessor::AppAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
   AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true))
#endif
{
}

AppAudioProcessor::~AppAudioProcessor()
{
}

const String AppAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AppAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AppAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AppAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
   return true;
   #else
   return false;
#endif
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

bool AppAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AppAudioProcessor::createEditor()
{   
    return new AppAudioProcessorEditor (*this);
}

/* Save the plugin state to the DAW project */
void AppAudioProcessor::getStateInformation (MemoryBlock& block)
{
}

/* Load the plugin state from the DAW project */
void AppAudioProcessor::setStateInformation (const void* in , int size)
{
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
   return new AppAudioProcessor();
}