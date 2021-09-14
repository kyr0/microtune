/***************************************************************
 ** 2021 Aron Homberg
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** AUDIOAPP IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

struct NoteMetadata {
    int octave;
    std::string noteName;
    int noteNumber;
};

class AppAudioProcessor : public foleys::MagicProcessor,
                          private juce::AudioProcessorValueTreeState::Listener
{
public:
   AppAudioProcessor();
    ~AppAudioProcessor() override;
   
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    NoteMetadata getNoteMetadata(int midiNoteNumber);
    int calculatePitchWheelValue(NoteMetadata& noteMetadata, int currentPitchWheelValue);

    // In this override you create the GUI ValueTree either using the default or loading from the BinaryData::magic_xml
    juce::ValueTree createGuiValueTree() override;

   // this is so that we can tell the host that something has changed, so that the host
   // can offer to save if the user hits exit.
   // AudioProcessor becomes the owner of this and will delete it
   AudioParameterFloat *mDummyParam;
   
private:
    juce::AudioProcessorValueTreeState treeState { *this, nullptr };

    float cCents;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
