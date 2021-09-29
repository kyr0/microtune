/***************************************************************
 ** Copyright (C) 2021 Aron Homberg
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** MICROTUNE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "Constants.h"
#include "PluginProcessor.h"
//#include "PluginEditor.h"
#include "BinaryData.h"
#include "PresetListBox.h"

const std::string WHITESPACE = " \n\r\t\f\v";

// trim from end (in place)
std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

namespace ParamIDs
{
    static juce::String cCents  { "cCents" };
    static juce::String cSharpCents    { "cSharpCents" };
    static juce::String dCents    { "dCents" };
    static juce::String dSharpCents    { "dSharpCents" };
    static juce::String eCents    { "eCents" };
    static juce::String fCents    { "fCents" };
    static juce::String fSharpCents    { "fSharpCents" };
    static juce::String gCents    { "gCents" };
    static juce::String gSharpCents    { "gSharpCents" };
    static juce::String aCents    { "aCents" };
    static juce::String aSharpCents    { "aSharpCents" };
    static juce::String bCents    { "bCents" };
}

namespace ToneNames
{
    static juce::String toneC  { "C" };
    static juce::String toneCSharp    { "C#" };
    static juce::String toneD    { "D" };
    static juce::String toneDSharp    { "D#" };
    static juce::String toneE    { "E" };
    static juce::String toneF    { "F" };
    static juce::String toneFSharp    { "F#" };
    static juce::String toneG    { "G" };
    static juce::String toneGSharp    { "G#" };
    static juce::String toneA    { "A" };
    static juce::String toneASharp    { "A#" };
    static juce::String toneB    { "B" };
}

namespace {
    constexpr int kWheelMiddlePosValue = 8192;
    constexpr int kWheelMaxValue = 16383;
    // max value divided by 198 cents because 200 cents make +/-2 semitones as by the MIDI spec,
    // but we're dealing with 0 indexed numbers (-1) and 0 itself is a zero-multiplier (-2)
    constexpr float kWheelValuePerCent = kWheelMaxValue / 198;
} // namespace


void createPitchParameterForTone(const String& paramId, const String& paramName, juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    layout.add(std::make_unique<juce::AudioParameterFloat> (
            paramId, paramName,
            juce::NormalisableRange<float> (-100, 100, 1),
            0,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value, 0); },
            [](const juce::String& text) { return text.getFloatValue(); }
    ));
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // adding all parameters to the layout so that they can be set, linked to the state and automated
    createPitchParameterForTone(ParamIDs::cCents, "C Tone cents", layout);
    createPitchParameterForTone(ParamIDs::cSharpCents, "C# Tone cents", layout);
    createPitchParameterForTone(ParamIDs::dCents, "D Tone cents", layout);
    createPitchParameterForTone(ParamIDs::dSharpCents, "D# Tone cents", layout);
    createPitchParameterForTone(ParamIDs::eCents, "E Tone cents", layout);
    createPitchParameterForTone(ParamIDs::fCents, "F Tone cents", layout);
    createPitchParameterForTone(ParamIDs::fSharpCents, "F# Tone cents", layout);
    createPitchParameterForTone(ParamIDs::gCents, "G Tone cents", layout);
    createPitchParameterForTone(ParamIDs::gSharpCents, "G# Tone cents", layout);
    createPitchParameterForTone(ParamIDs::aCents, "A Tone cents", layout);
    createPitchParameterForTone(ParamIDs::aSharpCents, "A# Tone cents", layout);
    createPitchParameterForTone(ParamIDs::bCents, "B Tone cents", layout);

    return layout;
}

NoteMetadata AppAudioProcessor::getNoteMetadata(int midiNoteNumber) {

    NoteMetadata noteMetadata;

    // raster: 2 chars per note name "C ", "C#", "D ", ...
    std::string notes = "C C#D D#E F F#G G#A A#B ";

    // calculating the relative position inside an octave (e.g. 63 (MIDI note number) % 12 (tones) is 3
    // having the distance from left (3 chars) is nice, but because every note name has 2 places like "C ",
    // the index needs to be multiplied by 2. So rel. position for substr, which cuts a subset of a string,
    // points to 6 for the MIDI note number 63. This actually points to the start pos of D# in the "notes" string.
    // From there on, substr cuts the 2 places out. "D#" or "E " e.g. is now passed to rtim which cuts away
    // whitespaces from the right, leaving us with "D#" and "E", the correctly
    // formatted note name for a given MIDI note number.
    noteMetadata.noteName = rtrim(notes.substr((midiNoteNumber % 12) * 2, 2));

    // every octave has 12 tones, if we split the available 127 notes (0 to 127 noteNumber MIDI standard)
    // into bunches of 12 notes (e.g. 127 / 12) we end up with the count of availanble octaves in the MIDI standard.
    // here, we do it relatively, to find the octave, the specific key pressed, is in, shifted by 1 octave (-1) down
    // because of modulo. The int cast makes sure, no decimal places are left over.
    noteMetadata.octave = (int) midiNoteNumber / 12 - 1;
    noteMetadata.noteNumber = midiNoteNumber;
    noteMetadata.isDefined = true;

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

    // setting all cent tuning values initially
    cCents = treeState.getRawParameterValue ("cCents")->load();
    cSharpCents = treeState.getRawParameterValue ("cSharpCents")->load();
    dCents = treeState.getRawParameterValue ("dCents")->load();
    dSharpCents = treeState.getRawParameterValue ("dSharpCents")->load();
    eCents = treeState.getRawParameterValue ("eCents")->load();
    fCents = treeState.getRawParameterValue ("fCents")->load();
    fSharpCents = treeState.getRawParameterValue ("fSharpCents")->load();
    gCents = treeState.getRawParameterValue ("gCents")->load();
    gSharpCents = treeState.getRawParameterValue ("gSharpCents")->load();
    aCents = treeState.getRawParameterValue ("aCents")->load();
    aSharpCents = treeState.getRawParameterValue ("aSharpCents")->load();
    bCents = treeState.getRawParameterValue ("bCents")->load();

    // preset handling
    presetList = magicState.createAndAddObject<PresetListBox>("presets");

    presetList->onSelectionChanged = [&](int index)
    {
        loadPresetInternal(index);

        currentPresetIndexSelected = index;
    };

    magicState.addTrigger ("save-preset", [this]
    {
        savePresetInternal();
    });

    magicState.addTrigger ("remove-preset", [this]
    {
        removePresetInternal(currentPresetIndexSelected);
    });

    magicState.setApplicationSettingsFile (juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                               .getChildFile (ProjectInfo::companyName)
                                               .getChildFile (ProjectInfo::projectName + juce::String (".settings")));
}

void AppAudioProcessor::savePresetInternal()
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);

    juce::ValueTree preset { "Preset" };

    auto name = magicState.getPropertyAsValue(":presetName").getValue().toString();

    if (name == "") {
        name = juce::String (presetNode.getNumChildren() + 1);
    }

    preset.setProperty ("name", name, nullptr);

    foleys::ParameterManager manager (*this);
    manager.saveParameterValues (preset);

    presetNode.appendChild (preset, nullptr);
}

void AppAudioProcessor::removePresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);
    auto preset = presetNode.getChild (index);

    presetNode.removeChild(index, nullptr);
}

void AppAudioProcessor::loadPresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName ("presets", nullptr);
    auto preset = presetNode.getChild (index);

    // set the label value (preset name)
    magicState.getPropertyAsValue(":presetName").setValue(preset.getProperty("name"));

    foleys::ParameterManager manager (*this);
    manager.loadParameterValues (preset);
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

    float toneValue;

    if (noteMetadata.noteName == ToneNames::toneC) {
        toneValue = cCents;
    }

    if (noteMetadata.noteName == ToneNames::toneCSharp) {
        toneValue = cSharpCents;
    }

    if (noteMetadata.noteName == ToneNames::toneD) {
        toneValue = dCents;
    }

    if (noteMetadata.noteName == ToneNames::toneDSharp) {
        toneValue = dSharpCents;
    }

    if (noteMetadata.noteName == ToneNames::toneE) {
        toneValue = eCents;
    }

    if (noteMetadata.noteName == ToneNames::toneF) {
        toneValue = fCents;
    }

    if (noteMetadata.noteName == ToneNames::toneFSharp) {
        toneValue = fSharpCents;
    }

    if (noteMetadata.noteName == ToneNames::toneG) {
        toneValue = gCents;
    }

    if (noteMetadata.noteName == ToneNames::toneGSharp) {
        toneValue = gSharpCents;
    }

    if (noteMetadata.noteName == ToneNames::toneA) {
        toneValue = aCents;
    }

    if (noteMetadata.noteName == ToneNames::toneASharp) {
        toneValue = aSharpCents;
    }

    if (noteMetadata.noteName == ToneNames::toneB) {
        toneValue = bCents;
    }

    auto nextWheelValue = (float) currentPitchWheelValue + (toneValue * kWheelValuePerCent);

    // hard limiting windowing for lower margin >= 0 upper margin <= 0x400
    if (nextWheelValue < 0) return 0;
    if (nextWheelValue >= 0x4000) return kWheelMaxValue;

    return (int) nextWheelValue;
}

void AppAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiBuffer)
{
   juce::MidiBuffer outputBuffer;

   // PitchWheel has a range from 0 to 16384
   // mean +- 2 semitones by general MIDI standard
   // temperament

   // clear all audio sample buffers
   // we don't produce any audio nor do we filter incoming audio
   buffer.clear();

    int sampleNumber = 0;

    int currentPitchWheelValue = kWheelMiddlePosValue;

    for (const auto midiBufferItem : midiBuffer) {

        auto midiMessage = midiBufferItem.getMessage();

        if (midiMessage.isNoteOn()) {

            currentNotePlayed = getNoteMetadata(midiBufferItem.getMessage().getNoteNumber());




            auto noteNumber = midiMessage.getNoteNumber();
            auto veolcity = midiMessage.getFloatVelocity();

            // queue noteOn
            auto onMessage =  juce::MidiMessage::noteOn(1, noteNumber, veolcity);
            onMessage.setTimeStamp(midiMessage.getTimeStamp());
            outputBuffer.addEvent(onMessage, sampleNumber);

            sampleNumber++;

            currentPitchWheelValue = calculatePitchWheelValue(currentNotePlayed, currentPitchWheelValue);

            // MIDI pitch adjustment message after each noteOn to make sure the pitch microtuning is in place
            auto pitchAdjustmentMessage = juce::MidiMessage::pitchWheel(midiMessage.getChannel(), currentPitchWheelValue);
            pitchAdjustmentMessage.setTimeStamp(midiMessage.getTimeStamp());
            outputBuffer.addEvent(pitchAdjustmentMessage, sampleNumber);

            sampleNumber++;
        }

        if (midiBufferItem.getMessage().isPitchWheel()) {

            auto newPitchWheelValue = midiBufferItem.getMessage().getPitchWheelValue();
            //pitchBendPercent = currentPitchWheelValue / kHighResolutionMax; // 0.5 -> mid, 0 -> low, 1 -> high
            int relativePitchWheelNoteDifference = 0;

            if (currentNotePlayed.isDefined) {
                relativePitchWheelNoteDifference = calculatePitchWheelValue(currentNotePlayed, kWheelMiddlePosValue) - kWheelMiddlePosValue;
            }
            currentPitchWheelValue = newPitchWheelValue + relativePitchWheelNoteDifference;

            auto pitchMessage = juce::MidiMessage::pitchWheel(
                    midiBufferItem.getMessage().getChannel(),
                // every pitch wheel movement must add the microtuning difference to be relatively correct
                    currentPitchWheelValue
            );
            pitchMessage.setTimeStamp(midiBufferItem.getMessage().getTimeStamp());
            outputBuffer.addEvent(pitchMessage, sampleNumber);

            sampleNumber++;
        }

        if (midiBufferItem.getMessage().isNoteOff()) {
            auto noteNumber = midiBufferItem.getMessage().getNoteNumber();
            auto velocity = midiBufferItem.getMessage().getFloatVelocity();

            // queue noteOff
            auto offMessage =  juce::MidiMessage::noteOff(1, noteNumber, velocity);
            offMessage.setTimeStamp(midiBufferItem.getMessage().getTimeStamp());
            outputBuffer.addEvent(offMessage, sampleNumber);

            // reset pitch wheel
            //currentPitchWheelValue = kWheelMiddlePosValue;

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
    if (paramId == ParamIDs::cCents) {
        cCents = newValue;
    }

    if (paramId == ParamIDs::cSharpCents) {
        cSharpCents = newValue;
    }

    if (paramId == ParamIDs::dCents) {
        dCents = newValue;
    }

    if (paramId == ParamIDs::dSharpCents) {
        dSharpCents = newValue;
    }

    if (paramId == ParamIDs::eCents) {
        eCents = newValue;
    }

    if (paramId == ParamIDs::fCents) {
        fCents = newValue;
    }

    if (paramId == ParamIDs::fSharpCents) {
        fSharpCents = newValue;
    }

    if (paramId == ParamIDs::gCents) {
        gCents = newValue;
    }

    if (paramId == ParamIDs::gSharpCents) {
        gSharpCents = newValue;
    }

    if (paramId == ParamIDs::aCents) {
        aCents = newValue;
    }

    if (paramId == ParamIDs::aSharpCents) {
        aSharpCents = newValue;
    }

    if (paramId == ParamIDs::bCents) {
        bCents = newValue;
    }
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
