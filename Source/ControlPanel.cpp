/*
  ==============================================================================

    ControlPanel.cpp
    Created: 18 Sep 2022 7:28:57pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "ControlPanel.h"

//==============================================================================
ControlPanel::ControlPanel (AudioFilePlayer& syncPlayer,
                            AudioFilePlayer& filePlayer,
                            AudioFormatManager& manager,
                            double maxLatencyInMs)
    : latencyPanel (syncPlayer, maxLatencyInMs),
      filePlayerPanel (filePlayer, manager)
{
    addAndMakeVisible (latencyPanel);
    addAndMakeVisible (filePlayerPanel);
}

void ControlPanel::resized()
{
    //==========================================================================
    // Manage panel hight
    int requiredHeight = latencyPanel.getHeight() + filePlayerPanel.getHeight();
    setSize (getWidth(), requiredHeight);

    // Get usable bounds
    auto bounds = getLocalBounds();

    // Set panel bounds
    latencyPanel.setBounds (bounds.removeFromTop (latencyPanel.getHeight()));
    filePlayerPanel.setBounds (bounds.removeFromTop (filePlayerPanel.getHeight()));
}

void ControlPanel::attachLatencyParameter (std::atomic<float>* latency)
{
    latencyPanel.attachLatencyParameter (latency);
}
