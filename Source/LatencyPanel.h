/*
  ==============================================================================

    LatencyPanel.h
    Created: 18 Sep 2022 7:27:42pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LatencyPanel  : public juce::Component
{
public:
    LatencyPanel();
    ~LatencyPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatencyPanel)
};
