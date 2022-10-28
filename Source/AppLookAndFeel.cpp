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
    const Colour outlineColour (0xFF6D6D6D);

    const Colour buttonOffColour (0xFF232323);
    const Colour buttonOnColour (0xFF181818);
    const Colour playColour (0xFF075E14);
    const Colour stopColour (0xFF952A09);

    const Colour sliderTrackColour (0xFFB87600);

    const Colour textColour (0xFFEFEFEF);
    const Colour headingColour (0xFFDFDFDF);

    // Window colours
    setColour (ResizableWindow::backgroundColourId, backgroundColour);

    // Label colours
    setColour (Label::textColourId, textColour);
    setColour (AppLookAndFeel::headingColourId, headingColour);

    // Button colours
    setColour (TextButton::buttonColourId, buttonOffColour);
    setColour (TextButton::buttonOnColourId, buttonOnColour);
    setColour (TextButton::textColourOnId, textColour);
    setColour (TextButton::textColourOffId, textColour);
    setColour (AppLookAndFeel::playButtonColourId, playColour);
    setColour (AppLookAndFeel::stopButtonColourId, stopColour);

    // Combobox colours
    setColour (ComboBox::outlineColourId, outlineColour);
    setColour (ComboBox::buttonColourId, buttonOffColour);
    setColour (ComboBox::backgroundColourId, buttonOffColour);
    setColour (ComboBox::textColourId, textColour);
    setColour (ComboBox::arrowColourId, textColour);

    setColour (PopupMenu::textColourId, headingColour);
    setColour (PopupMenu::highlightedTextColourId, textColour);
    setColour (PopupMenu::backgroundColourId, backgroundColour);
    setColour (PopupMenu::highlightedBackgroundColourId, buttonOnColour);

    // Slider colours
    setColour (Slider::textBoxOutlineColourId, outlineColour);
    setColour (Slider::textBoxTextColourId, textColour);
    setColour (Slider::textBoxBackgroundColourId, buttonOffColour);
    setColour (Slider::thumbColourId, headingColour);
    setColour (Slider::trackColourId, sliderTrackColour);
    setColour (Slider::backgroundColourId, sliderTrackColour.withAlpha (0.2f));

    // Scrollbar colours
    setColour (ScrollBar::thumbColourId, backgroundColour.brighter());

}
