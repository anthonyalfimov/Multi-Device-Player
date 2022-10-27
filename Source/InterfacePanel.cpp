/*
  ==============================================================================

    InterfacePanel.cpp
    Created: 18 Sep 2022 7:27:53pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include <JuceHeader.h>
#include "InterfacePanel.h"

//==============================================================================
void InterfacePanel::setSliderBounds (Slider& sl, Label& lb, juce::Rectangle<int> b)
{
    auto labelBounds = b.removeFromLeft (buttonWidth);
    lb.setBounds (labelBounds);

    b.removeFromLeft (padding);
    sl.setBounds (b);
}
