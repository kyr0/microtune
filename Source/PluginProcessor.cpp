/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
  constexpr int kMidiControlBits = 7;
  constexpr float kHighResolutionMax = (1 << (2 * kMidiControlBits)) - 1.0f;
  constexpr float kControlMax = (1 << kMidiControlBits) - 1.0f;
} // namespace

//==============================================================================
FireAtWillAudioProcessor::FireAtWillAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FireAtWillAudioProcessor::~FireAtWillAudioProcessor()
{
}

//==============================================================================
const juce::String FireAtWillAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FireAtWillAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FireAtWillAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FireAtWillAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FireAtWillAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FireAtWillAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FireAtWillAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FireAtWillAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FireAtWillAudioProcessor::getProgramName (int index)
{
    return {};
}

void FireAtWillAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FireAtWillAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    sampleRate = getSampleRate();
    
    auto pluginSettings = getPluginSettings(apvts);
    
}

void FireAtWillAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FireAtWillAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FireAtWillAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{

    for (const auto metadata : midiMessages) {
        
        
        if (metadata.getMessage().isNoteOn()) {
            auto noteNumber = metadata.getMessage().getNoteNumber();
            
            auto pitchMessage = juce::MidiMessage::pitchWheel(metadata.getMessage().getChannel(), 100);
            midiMessages.addEvent(pitchMessage, metadata.samplePosition + 1);
        }
        
        if (metadata.getMessage().isPitchWheel()) {
            // apply pitch difference value depending on the last note pressed
            auto currentPitchValue = metadata.getMessage().getPitchWheelValue();
            auto currentSamplePosition = metadata.samplePosition;
            midiMessages.clear(metadata.samplePosition, 1);
            
            auto pitchMessage = juce::MidiMessage::pitchWheel(metadata.getMessage().getChannel(), currentPitchValue + 100);
            midiMessages.addEvent(pitchMessage, currentSamplePosition);
        }
    }
    //juce::MidiBuffer fireAtWillMidiBuffer = *new juce::MidiBuffer;
    //juce::MidiBuffer pitchBuffer = *new juce::MidiBuffer;
    
    // PitchWheel has a range from 0 to 16384
    // mean +- 2 semitones by general MIDI standard
    // temperament
    
    /*
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto bufferLengthInMs = buffer.getNumSamples() / sampleRate * 1000;
    
    int sampleNumber = 0;
    
    // recently rememvered pitch value in Pct
    double pitchBendPercent;
    int pitchWheelChannel = 1;
    int pitchWheelValue = 0;
    
    for (const auto metadata : midiMessages) {
        
        if (metadata.getMessage().isPitchWheel()) {
            pitchWheelValue = metadata.getMessage().getPitchWheelValue();
            pitchBendPercent = pitchWheelValue / kHighResolutionMax; // 0.5 -> mid, 0 -> low, 1 -> high
            pitchWheelChannel = metadata.getMessage().getChannel();
            
            auto pitchMessage = juce::MidiMessage::pitchWheel(metadata.getMessage().getChannel(), metadata.getMessage().setChannel(<#int newChannelNumber#>))
            
            pitchBuffer.addEvent(<#const MidiMessage &midiMessage#>, <#int sampleNumber#>)
        }
        
        if (metadata.getMessage().isNoteOn()) {
            auto noteNumber = metadata.getMessage().getNoteNumber();
            auto veolcity = metadata.getMessage().getFloatVelocity();
            
            // queue noteOn
            auto onMessage =  juce::MidiMessage::noteOn(1,
                                                      noteNumber,
                                                      veolcity);
            
            
            fireAtWillMidiBuffer.addEvent(onMessage, sampleNumber);
            sampleNumber++;
        }
        
        if (metadata.getMessage().isNoteOff()) {
            auto noteNumber = metadata.getMessage().getNoteNumber();
            auto veolcity = metadata.getMessage().getFloatVelocity();
            
            // queue noteOff
            auto offMessage =  juce::MidiMessage::noteOff(1,
                                                      noteNumber,
                                                      veolcity);
            
            offMessage.setTimeStamp(metadata.getMessage().getTimeStamp());
            fireAtWillMidiBuffer.addEvent(offMessage, sampleNumber);
            
            sampleNumber++;
        }
    }
    
    // clear incoming messages - output shall be defined by the plugin only
    midiMessages.clear();
    
    sampleNumber = 0;
    
    
    
    // push the generated MIDI messages onto the output buffer
    for (const auto metadata : fireAtWillMidiBuffer) {
        sampleNumber++;
        
        // FIXME: not really doing anything?
        auto pitchWheelMessage = juce::MidiMessage::pitchWheel(metadata.getMessage().getChannel(), pitchWheelValue);
        pitchWheelMessage.setTimeStamp(metadata.getMessage().getTimeStamp());
        midiMessages.addEvent(pitchWheelMessage, sampleNumber);
        
        sampleNumber++;
        midiMessages.addEvent(metadata.getMessage(), sampleNumber);
    }
    
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
     */
}

//==============================================================================
bool FireAtWillAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FireAtWillAudioProcessor::createEditor()
{
    return new FireAtWillAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void FireAtWillAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void FireAtWillAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
        // TODO: update UI
    }
}

PluginSettings getPluginSettings(juce::AudioProcessorValueTreeState& apvts) {
    
    PluginSettings settings;
    
    settings.length = apvts.getRawParameterValue("Length Percent")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout FireAtWillAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Length Percent",
                                                         "Length Percent",
                                                         juce::NormalisableRange<float>(1.f, 100.f, 1.f, 1),
                                                         20.f));
    
    return layout;
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FireAtWillAudioProcessor();
}
