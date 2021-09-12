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

AppAudioProcessorEditor::AppAudioProcessorEditor(AppAudioProcessor &p)
    : AudioProcessorEditor(&p), mProcessor(p)
{

   // set main size. Note that this is done almost last, because it causes a paint to happen.
   // We don't want to paint on the items above until the above already happened, otherwise they
   // don't get another paint apparently
   setSize(600, 400);
}

AppAudioProcessorEditor::~AppAudioProcessorEditor()
{
   // avoid an assert
   setLookAndFeel(nullptr);
}

void AppAudioProcessorEditor::paint(Graphics &g)
{
   g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AppAudioProcessorEditor::resized()
{
   auto bounds = getLocalBounds();
}
