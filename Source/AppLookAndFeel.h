/*
  ==============================================================================

    AppLookAndFeel.h
    Created: 29 Sep 2022 3:47:50pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class AppLookAndFeel  : public LookAndFeel_V4
{
public:
    /**
        Creates and initialises AppLookAndFeel.
     */
    AppLookAndFeel();

    enum ColourIds
    {
        panelColourId = 1,
        headingColourId,
    };

private:

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppLookAndFeel)
};
