/*
  ==============================================================================

    ControlPanel.h
    Created: 18 Sep 2022 7:28:57pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InterfacePanel.h"
#include "LatencyPanel.h"
#include "FilePlayerPanel.h"

//==============================================================================

class ControlPanel  : public Component
{
public:
    ControlPanel (AudioFilePlayer& syncPlayer,
                  AudioFilePlayer& filePlayer,
                  AudioFormatManager& manager,
                  double maxLatency);

    void resized() override;

    //==========================================================================
    // Parameter access
    Slider& getLatency() { return latencyPanel.getLatency(); }
    bool isSyncTrackPlaying() const { return latencyPanel.isSyncTrackPlaying(); }

private:
    LatencyPanel latencyPanel;
    FilePlayerPanel filePlayerPanel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel);
};
