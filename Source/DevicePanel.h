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

//==============================================================================
class DevicePanel  : public InterfacePanel,
                     public ComponentListener
{
public:
    DevicePanel (AudioDeviceManager& main, AudioDeviceManager& linked);

    //==========================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==========================================================================
    void setDeviceSelectorEnabled (bool shouldBeEnabled);
    bool isDeviceSelectorEnabled() const;

    //==========================================================================
    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;

private:
    Label outputPanelLabel;

    //==========================================================================
    // Audio ouput devices
    AudioDeviceManager& mainDeviceManager;
    AudioDeviceSelectorComponent mainSelectorPanel;

    AudioDeviceManager& linkedDeviceManager;
    AudioDeviceSelectorComponent linkedSelectorPanel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DevicePanel)
};
