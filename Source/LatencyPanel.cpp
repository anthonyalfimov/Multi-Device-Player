/*
  ==============================================================================

    LatencyPanel.cpp
    Created: 18 Sep 2022 7:27:42pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LatencyPanel.h"

//==============================================================================
LatencyPanel::LatencyPanel (AudioFilePlayer& player, double maxLatency)
    : syncPlayer (player)
{
    // Latency panel label:
    addAndMakeVisible (latencyPanelLabel);
    latencyPanelLabel.setFont (headingFont);
    latencyPanelLabel.setColour (Label::textColourId, headingColour);
    latencyPanelLabel.setText("Latency Compensation", dontSendNotification);

    // Sync track button:
    addAndMakeVisible (syncTrackButton);
    syncTrackButton.setToggleable (true);
    syncTrackButton.setToggleState (false, dontSendNotification);
    syncTrackButton.setButtonText ("Play Sync Track");

    syncPlayer.onTransportStarted = [this]
    {
        syncTrackButton.setButtonText ("Stop Sync Track");
        syncTrackButton.setToggleState (true, dontSendNotification);
    };

    syncPlayer.onTransportStopped = [this]
    {
        syncTrackButton.setButtonText ("Play Sync Track");
        syncTrackButton.setToggleState (false, dontSendNotification);
    };

    syncTrackButton.onClick = [this]
    {
        if (! syncTrackButton.getToggleState())
            syncPlayer.playPause();
        else
            syncPlayer.stop();
    };

    // Latency slider
    addAndMakeVisible (latencySlider);
    addAndMakeVisible (latencySliderLabel);
    latencySlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
    latencySlider.setDoubleClickReturnValue (true, 0.0);
    latencySlider.setScrollWheelEnabled (false);
    latencySlider.setRange ({ -maxLatency, maxLatency }, 1.0);
    latencySlider.setTextValueSuffix (" ms");
    latencySliderLabel.setText ("Latency", dontSendNotification);
}

void LatencyPanel::resized()
{
    // Manage panel hight
    int requiredHeight = 3 * (buttonHeight + padding) + 2 * padding;
    setSize (getWidth(), requiredHeight);

    // Set control bounds
    auto bounds = getLocalBounds().reduced (padding / 2);

    latencyPanelLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                       .withTrimmedTop (padding)
                                       .withTrimmedLeft (padding));

    syncTrackButton.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                     .withTrimmedTop (padding)
                                     .withTrimmedLeft (padding)
                                     .withWidth (2 * buttonWidth + padding));

    setSliderBounds (latencySlider,
                     latencySliderLabel,
                     bounds.removeFromTop (buttonHeight + padding)
                           .withTrimmedTop (padding)
                           .withTrimmedLeft (padding));
}
