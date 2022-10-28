/*
  ==============================================================================

    InterfacePanel.h
    Created: 18 Sep 2022 7:27:53pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AppLookAndFeel.h"

//==============================================================================
/* TODO: Incorporate into the Look and Feel class
 */
class InterfacePanel  : public Component
{
public:
    InterfacePanel() = default;

    //==========================================================================
    // Interface constants
    static inline constexpr int padding = 12;

    static inline constexpr int buttonHeight = 25;
    static inline constexpr int buttonWidth = 100;

    static inline constexpr float corner = 5.0f;
    static inline constexpr float line = 2.0f;

protected:
    //==========================================================================
    // Interface fonts
    const Font headingFont { 18, Font::plain };

    //==========================================================================
    // Interface utilities
    static void setSliderBounds (Slider& sl, Label& lb, juce::Rectangle<int> b);

private:
    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterfacePanel)
};
