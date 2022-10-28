/*
  ==============================================================================

    OutputConfigurationPanel.cpp
    Created: 27 Oct 2022 7:11:18pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "OutputConfigPanel.h"

//==============================================================================
OutputConfigurationPanel::OutputConfigurationPanel (StringRef outputName,
                                                    AudioDeviceManager& adm,
                                                    bool showPhaseInvertOption,
                                                    std::function<void (float)> setGain)
    : manager (adm),
      selectorPanel (adm, 0, 0, 2, 2, false, false, true, false),
      showPhaseInvert (showPhaseInvertOption)
{
    //==========================================================================
    // Title
    addAndMakeVisible (outputLabel);
    outputLabel.setFont (headingFont);
    const auto headingColour
    = getLookAndFeel().findColour (AppLookAndFeel::headingColourId);
    outputLabel.setColour (Label::textColourId, headingColour);
    outputLabel.setText (outputName, dontSendNotification);

    //==========================================================================
    // Device selector
    addAndMakeVisible (selectorPanel);

    //==========================================================================
    // Volume control
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (volumeSliderLabel);
    volumeSlider.setTextBoxStyle (Slider::TextBoxRight, false,
                                  buttonWidth, buttonHeight);
    volumeSlider.setDoubleClickReturnValue (true, 0.0);
    volumeSlider.setScrollWheelEnabled (false);
    volumeSlider.setRange ({ 0.0, 100.0 }, 0.1);
    volumeSlider.setValue (25.0);
    volumeSlider.setTextValueSuffix (" %");
    volumeSliderLabel.setText ("Volume", dontSendNotification);

    auto setOutputGain = [this, setGain]()
    {
        const float absGain = static_cast<float> (volumeSlider.getValue() * 0.01);
        setGain (absGain * (phaseInvert.getToggleState() ? -1 : 1));
    };

    volumeSlider.onValueChange = setOutputGain;

    //==========================================================================
    // Phase control
    if (showPhaseInvert)
    {
        addAndMakeVisible (phaseInvert);
        phaseInvert.setButtonText ("Invert Phase");
        phaseInvert.onStateChange = setOutputGain;
    }
}

void OutputConfigurationPanel::resized()
{
    // Manage panel hight
    const int requiredHeight = selectorPanel.getHeight() + 2 * buttonHeight + 4 * padding
                             + (showPhaseInvert ? buttonHeight + padding : 0);
    setSize (getWidth(), requiredHeight);

    auto bounds = getLocalBounds().reduced (padding);   // get usable bounds

    // Output Label:
    outputLabel.setBounds (bounds.removeFromTop (buttonHeight));

    // Volume control:
    bounds.removeFromTop (padding);     // add spacing
    setSliderBounds (volumeSlider, volumeSliderLabel,
                     bounds.removeFromTop (buttonHeight));

    // Phase control:
    if (showPhaseInvert)
    {
        bounds.removeFromTop (padding);     // add spacing
        phaseInvert.setBounds (bounds.removeFromTop (buttonHeight)
                                     .withWidth (2 * buttonWidth));
    }

    // Device Selector:
    bounds.removeFromTop (padding);     // add spacing
    selectorPanel.setBounds (bounds.removeFromTop (selectorPanel.getHeight()));
}

void OutputConfigurationPanel::setDeviceSelectorEnabled (bool shouldBeEnabled)
{
    selectorPanel.setEnabled (shouldBeEnabled);
}

bool OutputConfigurationPanel::isDeviceSelectorEnabled() const
{
    return selectorPanel.isEnabled();
}
