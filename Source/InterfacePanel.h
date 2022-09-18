/*
  ==============================================================================

    InterfacePanel.h
    Created: 18 Sep 2022 7:27:53pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class InterfacePanel  : public juce::Component
{
public:
    InterfacePanel();
    ~InterfacePanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterfacePanel)
};
