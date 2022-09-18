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
void InterfacePanel::paint (juce::Graphics& g)
{
    auto bgColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.setColour (bgColour.darker (0.2f));

    auto bounds = getLocalBounds().reduced (padding / 2).toFloat();
    g.drawRoundedRectangle (bounds, corner, line);
}

void InterfacePanel::setSliderBounds (Slider& sl, Label& lb, juce::Rectangle<int> b)
{
    auto labelBounds = b.removeFromLeft (buttonWidth);
    lb.setBounds (labelBounds);

    b.removeFromLeft (padding);
    b.removeFromRight (padding);
    sl.setBounds (b);
}
