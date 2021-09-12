/***************************************************************
 ** Copyright (C) 2021 Aron Homberg
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** AUDIOAPP IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"

/* Main UI class */
class AppAudioProcessorEditor : public AudioProcessorEditor
{
   AppAudioProcessor& mProcessor;

public:

    // constructor
    AppAudioProcessorEditor (AppAudioProcessor&);

    // destructor
    ~AppAudioProcessorEditor();

    // paints the UI
    void paint (Graphics&) override;

    // called after resize to re-layout UI components
    void resized() override;

private:
   // don't know what this is right now so I leave it here
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppAudioProcessorEditor)
};

#endif  // PLUGINEDITOR_H_INCLUDED
