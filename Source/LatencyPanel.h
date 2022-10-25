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
    explicit LatencyPanel (AudioFilePlayer& player, double maxLatencyInMs);

    //==========================================================================
    void paint (Graphics& g) override {}
    void resized() override;

    //==========================================================================
    // Parameter attachment
    void attachLatencyParameter (std::atomic<float>* latency);

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
