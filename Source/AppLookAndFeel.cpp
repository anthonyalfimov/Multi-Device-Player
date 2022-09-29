/*
  ==============================================================================

    AppLookAndFeel.cpp
    Created: 29 Sep 2022 3:47:50pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "AppLookAndFeel.h"

AppLookAndFeel::AppLookAndFeel()
{
    // Colour constants
    const Colour backgroundColour (0xFF323232);
    const Colour panelColour (0xFF202020);

    const Colour textColour (0xFFEFEFEF);
    const Colour headingColour (0xFFDFDFDF);

    // Window colours
    setColour (ResizableWindow::backgroundColourId, backgroundColour);
    setColour (AppLookAndFeel::panelColourId, panelColour);

    // Text colours
    setColour (Label::textColourId, textColour);
    setColour (AppLookAndFeel::headingColourId, headingColour);

    // Button colours
    setColour (TextButton::buttonColourId, Colour (0xFF323232));
    setColour (TextButton::buttonOnColourId, Colour (0xFF202020));

    // Slider colours
    setColour (ScrollBar::thumbColourId, backgroundColour.brighter());
}
