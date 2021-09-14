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
//#include "PluginEditor.h"
#include "BinaryData.h"

namespace IDs
{
    // TODO: Add all other tones
    static juce::String cCents  { "cCents" };
    static juce::String cSharpCents    { "cSharpCents" };
}

namespace {
    constexpr int kWheelMiddlePosValue = 8192;
    constexpr int kWheelMaxValue = 16383;
    // max value divided by 198 cents because 200 cents make +/-2 semitones as by the MIDI spec,
    // but we're dealing with 0 indexed numbers (-1) and 0 itself is a zero-multiplier (-2)
    constexpr float kWheelValuePerCent = kWheelMaxValue / 198;
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // TODO: Add all other tones

    layout.add(std::make_unique<juce::AudioParameterFloat> (
            IDs::cCents, "cCents",
            juce::NormalisableRange<float> (-100, 100, 1),
            0,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value, 0); },
            [](juce::String text) { return text.getFloatValue(); }
    ));

    return layout;
}

NoteMetadata AppAudioProcessor::getNoteMetadata(int midiNoteNumber) {

    NoteMetadata noteMetadata;

    std::string notes = "C C#D D#E F F#G G#A A#B ";

    noteMetadata.noteName = notes.substr((midiNoteNumber % 12) * 2, 2);
    noteMetadata.octave = midiNoteNumber / 12 - 1;
    noteMetadata.noteNumber = midiNoteNumber;

    return noteMetadata;
}

AppAudioProcessor::AppAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
   MagicProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true)),
#endif
   treeState (*this, nullptr, JucePlugin_Name, createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH (__FILE__);

    // listen for parameter changes
    for (auto* parameter : getParameters())
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter))
            treeState.addParameterListener (p->paramID, this);

    // TODO: Adapt from: https://github.com/ffAudio/PluginGuiMagic/blob/main/examples/EqualizerExample/Source/PluginProcessor.cpp#L63

    cCents = treeState.getRawParameterValue ("cCents")->load();
}

AppAudioProcessor::~AppAudioProcessor()
{
    for (auto* parameter : getParameters())
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(parameter))
            treeState.removeParameterListener (p->paramID, this);
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

int AppAudioProcessor::calculatePitchWheelValue(NoteMetadata& noteMetadata, int currentPitchWheelValue) {

    // TODO: select the right nextWheelValue to multiply (based on noteMetadata)
    auto nextWheelValue = currentPitchWheelValue + (cCents * kWheelValuePerCent);

    // hard limiting windowing for lower margin >= 0 upper margin <= 0x400
    if (nextWheelValue < 0) return 0;
    if (nextWheelValue >= 0x4000) return kWheelMaxValue;

    return nextWheelValue;
}

void AppAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiBuffer)
{
   juce::MidiBuffer outputBuffer;
   //juce::MidiBuffer pitchBuffer;

   // PitchWheel has a range from 0 to 16384
   // mean +- 2 semitones by general MIDI standard
   // temperament

   // clear all audio sample buffers
   // we don't produce any audio nor do we filter incoming audio
   buffer.clear();

    int sampleNumber = 0;

    // recently rememvered pitch value in Pct
    //float pitchBendPercent;
    //int pitchWheelChannel = 1;
    int currentPitchWheelValue = kWheelMiddlePosValue;
    //int currentMidiNote;

    for (const auto midiBufferItem : midiBuffer) {

        auto noteMetaData = getNoteMetadata(midiBufferItem.getMessage().getNoteNumber());
        auto midiMessage = midiBufferItem.getMessage();

        if (midiMessage.isNoteOn()) {
            auto noteNumber = midiMessage.getNoteNumber();
            auto veolcity = midiMessage.getFloatVelocity();

            // queue noteOn
            auto onMessage =  juce::MidiMessage::noteOn(1, noteNumber, veolcity);
            onMessage.setTimeStamp(midiMessage.getTimeStamp());
            outputBuffer.addEvent(onMessage, sampleNumber);

            sampleNumber++;

            auto nextPitchWheelValue = calculatePitchWheelValue(noteMetaData, currentPitchWheelValue);

            // MIDI pitch adjustment message after each noteOn to make sure the pitch microtuning is in place
            auto pitchAdjustmentMessage = juce::MidiMessage::pitchWheel(midiMessage.getChannel(), nextPitchWheelValue);
            pitchAdjustmentMessage.setTimeStamp(midiMessage.getTimeStamp());
            outputBuffer.addEvent(pitchAdjustmentMessage, sampleNumber);

            sampleNumber++;
        }

        if (midiBufferItem.getMessage().isPitchWheel()) {
            currentPitchWheelValue = midiBufferItem.getMessage().getPitchWheelValue();
            //pitchBendPercent = currentPitchWheelValue / kHighResolutionMax; // 0.5 -> mid, 0 -> low, 1 -> high
            //pitchWheelChannel = midiBufferItem.getMessage().getChannel();

            auto nextPitchWheelValue = calculatePitchWheelValue(noteMetaData, currentPitchWheelValue);

            auto pitchMessage = juce::MidiMessage::pitchWheel(
                    midiBufferItem.getMessage().getChannel(),
                // every pitch wheel movement must add the microtuning difference to be relatively correct
                    nextPitchWheelValue
            );
            pitchMessage.setTimeStamp(midiBufferItem.getMessage().getTimeStamp());
            outputBuffer.addEvent(pitchMessage, sampleNumber);

            sampleNumber++;
        }

        if (midiBufferItem.getMessage().isNoteOff()) {
            auto noteNumber = midiBufferItem.getMessage().getNoteNumber();
            auto veolcity = midiBufferItem.getMessage().getFloatVelocity();

            // queue noteOff
            auto offMessage =  juce::MidiMessage::noteOff(1, noteNumber, veolcity);
            offMessage.setTimeStamp(midiBufferItem.getMessage().getTimeStamp());
            outputBuffer.addEvent(offMessage, sampleNumber);

            sampleNumber++;
        }
    }

    // clear incoming messages - output shall be defined by the plugin only
    midiBuffer.clear();

    sampleNumber = 0;

    // re-adding to the buffer in the right order
    for (const auto midiMessage : outputBuffer) {
        sampleNumber++;
        midiBuffer.addEvent(midiMessage.getMessage(), sampleNumber);
    }
}

void AppAudioProcessor::parameterChanged (const juce::String& paramId, float newValue)
{
    if (paramId == IDs::cCents) {
        cCents = newValue;
    }

    // TODO: Add all other tones
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
