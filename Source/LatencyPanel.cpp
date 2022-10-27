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
LatencyPanel::LatencyPanel (AudioFilePlayer& player, double maxLatencyInMs)
    : syncPlayer (player)
{
    // Latency panel label:
    addAndMakeVisible (latencyPanelLabel);
    latencyPanelLabel.setFont (headingFont);
    const auto headingColour
    = getLookAndFeel().findColour (AppLookAndFeel::headingColourId);
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
    latencySlider.setTextBoxStyle (Slider::TextBoxRight, false,
                                   buttonWidth, buttonHeight);
    latencySlider.setDoubleClickReturnValue (true, 0.0);
    latencySlider.setScrollWheelEnabled (false);
    latencySlider.setRange ({ -maxLatencyInMs, maxLatencyInMs }, 1.0);
    latencySlider.setTextValueSuffix (" ms");
    latencySliderLabel.setText ("Latency", dontSendNotification);
}

void LatencyPanel::resized()
{
    // Manage panel hight
    int requiredHeight = 3 * (buttonHeight + padding) + padding;
    setSize (getWidth(), requiredHeight);

    auto bounds = getLocalBounds().reduced (padding);   // get usable bounds

    // Section label:
    latencyPanelLabel.setBounds (bounds.removeFromTop (buttonHeight));

    // Sync Track button:
    bounds.removeFromTop (padding);     // add spacing
    syncTrackButton.setBounds (bounds.removeFromTop (buttonHeight)
                                     .withWidth (2 * buttonWidth + padding));

    // Latency slider:
    bounds.removeFromTop (padding);     // add spacing
    setSliderBounds (latencySlider,
                     latencySliderLabel,
                     bounds.removeFromTop (buttonHeight));
}

void LatencyPanel::attachLatencyParameter (std::atomic<float>* latency)
{
    latencySlider.onValueChange = [latency, this]
    {
        latency->store (static_cast<float> (latencySlider.getValue()));
    };
}
