/*
  ==============================================================================

    OutputConfigPanel.h
    Created: 27 Oct 2022 7:11:18pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "InterfacePanel.h"

//==============================================================================
class OutputConfigurationPanel  : public InterfacePanel
{
public:
    OutputConfigurationPanel (AudioDeviceManager& adm, StringRef outputName,
                              bool showPhaseInvertOption = false);

    //==========================================================================
    void resized() override;

    //==========================================================================
    void setDeviceSelectorEnabled (bool shouldBeEnabled);
    bool isDeviceSelectorEnabled() const;

private:
    Label outputLabel;

    //==========================================================================
    // Device selector
    AudioDeviceManager& manager;
    AudioDeviceSelectorComponent selectorPanel;

    //==========================================================================
    // Volume control
    Slider volumeSlider;
    Label volumeSliderLabel;

    //==========================================================================
    // Phase control
    bool showPhaseInvert = false;
    ToggleButton phaseInvert;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputConfigurationPanel)
};
