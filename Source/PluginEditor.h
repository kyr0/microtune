#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider(): juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                       juce::Slider::TextEntryBoxPosition::TextBoxBelow)
    {
            
    }
};


class FireAtWillAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public juce::MidiKeyboardState::Listener
{
public:
    FireAtWillAudioProcessorEditor (FireAtWillAudioProcessor& p);
    
    ~FireAtWillAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void setupKeyboard();
    void setupMidiMonitor();
    
    void handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

private:
    FireAtWillAudioProcessor& audioProcessor;
    
    // keyboard component to render
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;
    
    CustomRotarySlider detuneSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment detuneSliderAttachment;
    
    std::vector<juce::Component*> getComponents();
    
    juce::TextEditor midiMonitor  { "MIDI Monitor" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FireAtWillAudioProcessorEditor)
};
