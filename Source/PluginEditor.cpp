#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FireAtWillAudioProcessorEditor::FireAtWillAudioProcessorEditor (FireAtWillAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), keyboardComponent(keyboardState,
                                                                       juce::MidiKeyboardComponent::horizontalKeyboard),
    detuneSliderAttachment(audioProcessor.apvts, "Length Percent", detuneSlider)
{
    
    setupKeyboard();
    
    setupMidiMonitor();
    
    
    
    for (auto* comp : getComponents()) {
        addAndMakeVisible(comp);
    }
    
    
    
    setSize(600, 400);
    
}

void FireAtWillAudioProcessorEditor::setupMidiMonitor()
{
    midiMonitor.setText("Version 0.0.1a");
}

void FireAtWillAudioProcessorEditor::setupKeyboard()
{
    keyboardComponent.setTitle("Click on notes to define their relative tuning:");
    keyboardComponent.setAvailableRange(0, 11);
    
    // route actual mouse action on the keyboard to the respective handleNoteOn / handleNoteOff virtual functions
    keyboardState.addListener(this);
}

void FireAtWillAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
   
    // TODO: Have fun setting the right slider
    
    
    
    
    resized();
}

void FireAtWillAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    juce::MidiMessage m (juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity));
    m.setTimeStamp (juce::Time::getMillisecondCounterHiRes() * 0.001);
   
    // TODO: Have fun setting the right slider
    resized();
}

FireAtWillAudioProcessorEditor::~FireAtWillAudioProcessorEditor()
{
}

void FireAtWillAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void FireAtWillAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    keyboardComponent.setKeyWidth(bounds.getWidth() / 7);
    
    //bounds.removeFromTop(bounds.getHeight() * 0.1);
    
    
    keyboardComponent.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() * 0.33);
    
    bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    midiMonitor.setBounds(bounds);
    
    //auto keyboardArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    
    detuneSlider.setBounds(bounds);
    
    
    
}

std::vector<juce::Component*> FireAtWillAudioProcessorEditor::getComponents()
{
    return {
        &detuneSlider,
        &keyboardComponent,
        &midiMonitor
    };
}
