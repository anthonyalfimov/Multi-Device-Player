/*
  ==============================================================================

    LatencyPanel.h
    Created: 18 Sep 2022 7:27:42pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InterfacePanel.h"
#include "AudioFilePlayer.h"

//==============================================================================
class LatencyPanel  : public InterfacePanel
{
public:
    LatencyPanel (AudioFilePlayer& player, double maxLatencyInMs,
                  std::function<void (float)> latencySetter);

    //==========================================================================
    void resized() override;

private:
    AudioFilePlayer& syncPlayer;

    //==========================================================================
    // UI Components
    Label latencyPanelLabel;
    TextButton syncTrackButton;
    Slider latencySlider;
    Label latencySliderLabel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyPanel)
};
