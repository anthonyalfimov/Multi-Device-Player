/*
  ==============================================================================

    AudioPlayerPanel.h
    Created: 18 Sep 2022 7:28:15pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class AudioPlayerPanel  : public juce::Component
{
public:
    AudioPlayerPanel();
    ~AudioPlayerPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerPanel)
};
