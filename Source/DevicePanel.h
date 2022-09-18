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
    explicit DevicePanel (AudioDeviceManager& manager);

    //==========================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==========================================================================
    void componentMovedOrResized (Component&, bool wasMoved, bool wasResized) override;

private:
    Label outputPanelLabel;

    //==========================================================================
    // Audio ouput device
    AudioDeviceManager& deviceManager;
    AudioDeviceSelectorComponent selectorPanel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DevicePanel)
};
