/*
  ==============================================================================

    DevicePanel.cpp
    Created: 18 Sep 2022 7:28:47pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DevicePanel.h"

//==============================================================================
DevicePanel::DevicePanel (AudioDeviceManager& manager)
    : deviceManager (manager),
      selectorPanel (deviceManager, 0, 0, 0, 2, false, false, true, false)
{
    //==========================================================================
    // Audio device section
    addAndMakeVisible (outputPanelLabel);
    outputPanelLabel.setFont (headingFont);
    outputPanelLabel.setColour (Label::textColourId, headingColour);
    outputPanelLabel.setText("Audio Output Configuration", dontSendNotification);

    addAndMakeVisible (selectorPanel);
    selectorPanel.addComponentListener (this);
}

//==============================================================================
void DevicePanel::paint (Graphics& g)
{
    auto bounds = getLocalBounds().reduced (padding / 2).toFloat();

    auto bgColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.setColour (bgColour.darker (0.2f));

    g.fillRoundedRectangle (bounds, corner);
    g.drawRoundedRectangle (bounds, corner, line);
}

void DevicePanel::resized()
{
    //==========================================================================
    // Manage panel hight
    int requiredHeight
    = selectorPanel.getHeight() + (buttonHeight + padding) + 3 * padding;
    setSize (getWidth(), requiredHeight);

    // Get usable bounds
    auto bounds = getLocalBounds().reduced (padding / 2);

    //==========================================================================
    // Audio device section
    outputPanelLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                      .withTrimmedTop (padding)
                                      .withTrimmedLeft (padding));

    bounds.removeFromTop (padding);   // add spacing

    selectorPanel.setBounds (bounds.removeFromTop (selectorPanel.getHeight()));
}

//==============================================================================
void DevicePanel::componentMovedOrResized (Component& component,
                                           bool wasMoved,
                                           bool wasResized)
{
    if (&component == &selectorPanel && wasResized)
        resized();
}
