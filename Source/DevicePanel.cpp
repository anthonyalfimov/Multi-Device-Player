/*
  ==============================================================================

    DevicePanel.cpp
    Created: 18 Sep 2022 7:28:47pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DevicePanel.h"

//==============================================================================
DevicePanel::DevicePanel (AudioDeviceManager& main, AudioDeviceManager& linked,
                          AudioFilePlayer& syncPlayer, double maxLatencyInMs)
    : mainDeviceManager (main),
      mainSelectorPanel (mainDeviceManager, 0, 0, 2, 2, false, false, true, false),
      linkedDeviceManager (linked),
      linkedSelectorPanel (linkedDeviceManager, 0, 0, 2, 2, false, false, true, false),
      latencyPanel (syncPlayer, maxLatencyInMs)
{
    //==========================================================================
    // Audio device section
    addAndMakeVisible (devicePanelLabel);
    devicePanelLabel.setFont (headingFont);
    const auto headingColour
    = getLookAndFeel().findColour (AppLookAndFeel::headingColourId);
    devicePanelLabel.setColour (Label::textColourId, headingColour);
    devicePanelLabel.setText("Audio Output Configuration", dontSendNotification);

    addAndMakeVisible (mainSelectorPanel);
    mainSelectorPanel.addComponentListener (this);

    addAndMakeVisible (linkedSelectorPanel);
    linkedSelectorPanel.addComponentListener (this);

    //==========================================================================
    // Latency compensation section
    addAndMakeVisible (latencyPanel);
}

//==============================================================================
void DevicePanel::paint (Graphics& g)
{
    auto bounds = getLocalBounds().reduced (padding / 2).toFloat();

    auto bgColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.setColour (bgColour.darker (0.2f));

    g.fillRoundedRectangle (bounds, corner);
    g.drawRoundedRectangle (bounds, corner, line);
}

void DevicePanel::resized()
{
    //==========================================================================
    // Manage panel hight
    int requiredHeight = mainSelectorPanel.getHeight() + linkedSelectorPanel.getHeight()
                       + latencyPanel.getHeight() + (buttonHeight + padding)
                       + 3 * padding;
    setSize (getWidth(), requiredHeight);

    // Get usable bounds
    auto bounds = getLocalBounds().reduced (padding / 2);

    //==========================================================================
    // Audio device section
    devicePanelLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                      .withTrimmedTop (padding)
                                      .withTrimmedLeft (padding));

    bounds.removeFromTop (padding);   // add spacing

    mainSelectorPanel.setBounds (bounds.removeFromTop (mainSelectorPanel.getHeight()));
    linkedSelectorPanel.setBounds (bounds.removeFromTop (linkedSelectorPanel
                                                         .getHeight()));

    //==========================================================================
    // Latency compensation section
    latencyPanel.setBounds (bounds.removeFromTop (latencyPanel.getHeight()));
}

//==============================================================================
void DevicePanel::attachLatencyParameter (std::atomic<float>* latency)
{
    latencyPanel.attachLatencyParameter (latency);
}

//==============================================================================
void DevicePanel::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    mainSelectorPanel.setEnabled (shouldBeEnabled);
    linkedSelectorPanel.setEnabled (shouldBeEnabled);
}

bool DevicePanel::isDeviceSelectorEnabled() const
{
    return mainSelectorPanel.isEnabled() && linkedSelectorPanel.isEnabled();
}

//==============================================================================
void DevicePanel::componentMovedOrResized (Component& component,
                                           bool wasMoved,
                                           bool wasResized)
{
    if (wasResized)
        resized();
}
