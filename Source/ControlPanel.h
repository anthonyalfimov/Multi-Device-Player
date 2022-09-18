/*
  ==============================================================================

    ControlPanel.h
    Created: 18 Sep 2022 7:28:57pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ControlPanel  : public juce::Component
{
public:
    ControlPanel();
    ~ControlPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
