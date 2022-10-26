/*
  ==============================================================================

    DevicePanel.cpp
    Created: 18 Sep 2022 7:28:47pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DevicePanel.h"

//==============================================================================
DeviceSettingsView::DeviceSettingsView (AudioDeviceManager& main,
                                        AudioDeviceManager& linked,
                                        AudioFilePlayer& syncPlayer,
                                        double maxLatencyInMs)
    : mainDeviceManager (main),
      mainSelectorPanel (mainDeviceManager, 0, 0, 2, 2, false, false, true, false),
      linkedDeviceManager (linked),
      linkedSelectorPanel (linkedDeviceManager, 0, 0, 2, 2, false, false, true, false),
      latencyPanel (syncPlayer, maxLatencyInMs)
{
    //==========================================================================
    // Audio device section
    const auto headingColour
    = getLookAndFeel().findColour (AppLookAndFeel::headingColourId);

    addAndMakeVisible (mainDeviceLabel);
    mainDeviceLabel.setFont (headingFont);
    mainDeviceLabel.setColour (Label::textColourId, headingColour);
    mainDeviceLabel.setText("Primary Output Device", dontSendNotification);

    addAndMakeVisible (mainSelectorPanel);
    mainSelectorPanel.addComponentListener (this);

    addAndMakeVisible (linkedDeviceLabel);
    linkedDeviceLabel.setFont (headingFont);
    linkedDeviceLabel.setColour (Label::textColourId, headingColour);
    linkedDeviceLabel.setText("Secondary Output Device", dontSendNotification);

    addAndMakeVisible (linkedSelectorPanel);
    linkedSelectorPanel.addComponentListener (this);

    //==========================================================================
    // Latency compensation section
    addAndMakeVisible (latencyPanel);
}

//==============================================================================
void DeviceSettingsView::resized()
{
    //==========================================================================
    // Manage panel hight
    int requiredHeight = mainSelectorPanel.getHeight() + linkedSelectorPanel.getHeight()
                       + latencyPanel.getHeight() + 2 * (buttonHeight + padding)
                       + 3 * padding;
    setSize (getWidth(), requiredHeight);

    // Get usable bounds
    auto bounds = getLocalBounds().reduced (padding / 2);

    //==========================================================================
    // Audio device section
    mainDeviceLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                     .withTrimmedTop (padding)
                                     .withTrimmedLeft (padding));
    bounds.removeFromTop (padding);   // add spacing
    mainSelectorPanel.setBounds (bounds.removeFromTop (mainSelectorPanel.getHeight()));

    linkedDeviceLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                       .withTrimmedTop (padding)
                                       .withTrimmedLeft (padding));
    bounds.removeFromTop (padding);   // add spacing
    linkedSelectorPanel.setBounds (bounds.removeFromTop (linkedSelectorPanel
                                                         .getHeight()));

    //==========================================================================
    // Latency compensation section
    latencyPanel.setBounds (bounds.removeFromTop (latencyPanel.getHeight()));
}

//==============================================================================
void DeviceSettingsView::attachLatencyParameter (std::atomic<float>* latency)
{
    latencyPanel.attachLatencyParameter (latency);
}

//==============================================================================
void DeviceSettingsView::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    mainSelectorPanel.setEnabled (shouldBeEnabled);
    linkedSelectorPanel.setEnabled (shouldBeEnabled);
}

bool DeviceSettingsView::isDeviceSelectorEnabled() const
{
    return mainSelectorPanel.isEnabled() && linkedSelectorPanel.isEnabled();
}

//==============================================================================
void DeviceSettingsView::componentMovedOrResized (Component& component,
                                                  bool wasMoved,
                                                  bool wasResized)
{
    if (wasResized)
        resized();
}

//==============================================================================
DevicePanel::DevicePanel (AudioDeviceManager& main, AudioDeviceManager& linked,
                          AudioFilePlayer& syncPlayer, double maxLatencyInMs)
    : deviceSettings (main, linked, syncPlayer, maxLatencyInMs)
{
    addAndMakeVisible (devicePanelViewport);
    devicePanelViewport.setViewedComponent (&deviceSettings, false);
    devicePanelViewport.setScrollBarsShown (true, false);
}

//==============================================================================
void DevicePanel::resized()
{
    deviceSettings.resized();

    const auto bounds = getLocalBounds();
    devicePanelViewport.setBounds (bounds);
    deviceSettings.setSize (bounds.getWidth(), deviceSettings.getHeight());

//    if (devicePanelViewport.getViewHeight() < devicePanel->getHeight())
//        devicePanelViewport.setBounds (bounds.withTrimmedRight (-3));
}

//==============================================================================
void DevicePanel::attachLatencyParameter (std::atomic<float>* latency)
{
    deviceSettings.attachLatencyParameter (latency);
}

//==============================================================================
void DevicePanel::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    deviceSettings.setDeviceSelectorEnabled (shouldBeEnabled);
}

bool DevicePanel::isDeviceSelectorEnabled() const
{
    return deviceSettings.isDeviceSelectorEnabled();
}
