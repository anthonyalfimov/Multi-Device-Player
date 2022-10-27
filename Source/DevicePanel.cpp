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
    : mainDevicePanel (main, "Primary Output Device"),
      linkedDevicePanel (linked, "Secondary Output Device", true),
      latencyPanel (syncPlayer, maxLatencyInMs)
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

void DeviceSettingsView::attachLatencyParameter (std::atomic<float>* latency)
{
    latencyPanel.attachLatencyParameter (latency);
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
DevicePanel::DevicePanel (AudioDeviceManager& main, AudioDeviceManager& linked,
                          AudioFilePlayer& syncPlayer, double maxLatencyInMs)
    : deviceSettings (main, linked, syncPlayer, maxLatencyInMs)
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

void DevicePanel::attachLatencyParameter (std::atomic<float>* latency)
{
    deviceSettings.attachLatencyParameter (latency);
}

void DevicePanel::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    deviceSettings.setDeviceSelectorEnabled (shouldBeEnabled);
}

bool DevicePanel::isDeviceSelectorEnabled() const
{
    return deviceSettings.isDeviceSelectorEnabled();
}
