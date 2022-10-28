/*
  ==============================================================================

    DevicePanel.h
    Created: 18 Sep 2022 7:28:47pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InterfacePanel.h"
#include "OutputConfigPanel.h"
#include "LatencyPanel.h"

//==============================================================================
class DeviceSettingsView  : public Component
{
public:
    DeviceSettingsView (AudioDeviceManager& main, AudioDeviceManager& linked,
                        AudioFilePlayer& syncPlayer, double maxLatencyInMs);

    //==========================================================================
    void resized() override;

    //==========================================================================
    // Parameter attachment
    void attachLatencyParameter (std::atomic<float>* latency);

    //==========================================================================
    void setDeviceSelectorEnabled (bool shouldBeEnabled);
    bool isDeviceSelectorEnabled() const;

private:
    OutputConfigurationPanel mainDevicePanel;
    OutputConfigurationPanel linkedDevicePanel;
    LatencyPanel latencyPanel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceSettingsView)
};

//==============================================================================
class DevicePanel  : public Component
{
public:
    DevicePanel (AudioDeviceManager& main, AudioDeviceManager& linked,
                 AudioFilePlayer& syncPlayer, double maxLatencyInMs);

    //==========================================================================
    void resized() override;

    //==========================================================================
    // Parameter attachment
    void attachLatencyParameter (std::atomic<float>* latency);

    //==========================================================================
    void setDeviceSelectorEnabled (bool shouldBeEnabled);
    bool isDeviceSelectorEnabled() const;

private:
    DeviceSettingsView deviceSettings;
    Viewport devicePanelViewport;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DevicePanel)
};
