/*
  ==============================================================================

    DevicePanel.cpp
    Created: 18 Sep 2022 7:28:47pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DevicePanel.h"

//==============================================================================
DeviceSettingsView::DeviceSettingsView (MultiDevicePlayer& mpd,
                                        AudioFilePlayer& syncPlayer,
                                        double maxLatencyInMs)
    : mainDevicePanel ("Primary Output Device", mpd.mainDeviceManager, false,
                       [&mpd] (float newGain) { mpd.setMainGain (newGain); }),
      linkedDevicePanel ("Secondary Output Device", mpd.linkedDeviceManager, true,
                         [&mpd] (float newGain) { mpd.setLinkedGain (newGain); }),
      latencyPanel (syncPlayer, maxLatencyInMs,
                    [&mpd] (float newLatency) { mpd.setLatency (newLatency); })
{
    addAndMakeVisible (mainDevicePanel);
    addAndMakeVisible (linkedDevicePanel);
    addAndMakeVisible (latencyPanel);
}

void DeviceSettingsView::resized()
{
    // Manage panel hight
    int requiredHeight = mainDevicePanel.getHeight()
                       + linkedDevicePanel.getHeight()
                       + latencyPanel.getHeight();
    setSize (getWidth(), requiredHeight);

    auto bounds = getLocalBounds();     // get usable bounds

    mainDevicePanel.setBounds (bounds.removeFromTop (mainDevicePanel.getHeight()));
    linkedDevicePanel.setBounds (bounds.removeFromTop (linkedDevicePanel.getHeight()));
    latencyPanel.setBounds (bounds.removeFromTop (latencyPanel.getHeight()));
}

void DeviceSettingsView::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    mainDevicePanel.setDeviceSelectorEnabled (shouldBeEnabled);
    linkedDevicePanel.setDeviceSelectorEnabled (shouldBeEnabled);
}

bool DeviceSettingsView::isDeviceSelectorEnabled() const
{
    return mainDevicePanel.isDeviceSelectorEnabled()
           && linkedDevicePanel.isDeviceSelectorEnabled();
}

//==============================================================================
DevicePanel::DevicePanel (MultiDevicePlayer& multiDevice, AudioFilePlayer& syncPlayer,
                          double maxLatencyInMs)
    : deviceSettings (multiDevice, syncPlayer, maxLatencyInMs)
{
    addAndMakeVisible (devicePanelViewport);
    devicePanelViewport.setViewedComponent (&deviceSettings, false);
    devicePanelViewport.setScrollBarsShown (true, false);
}

void DevicePanel::resized()
{
    deviceSettings.resized();

    const auto bounds = getLocalBounds();
    devicePanelViewport.setBounds (bounds);
    deviceSettings.setSize (bounds.getWidth(), deviceSettings.getHeight());
}

void DevicePanel::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    deviceSettings.setDeviceSelectorEnabled (shouldBeEnabled);
}

bool DevicePanel::isDeviceSelectorEnabled() const
{
    return deviceSettings.isDeviceSelectorEnabled();
}
