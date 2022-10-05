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
    explicit LatencyPanel (AudioFilePlayer& player, double maxLatency);

    void resized() override;

    //==========================================================================
    // Parameter access
    std::atomic<float>* getLatency() { return &latencyValue; }
    // TODO: Should this method use an atomic as well?
    bool isSyncTrackPlaying() const { return syncTrackButton.getToggleState(); }

private:
    AudioFilePlayer& syncPlayer;

    Label latencyPanelLabel;
    TextButton syncTrackButton;
    Slider latencySlider;
    Label latencySliderLabel;

    std::atomic<float> latencyValue { 0 };

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyPanel)
};
