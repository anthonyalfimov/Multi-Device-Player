/*
  ==============================================================================

    FilePlayerPanel.cpp
    Created: 18 Sep 2022 7:28:15pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "FilePlayerPanel.h"

//==============================================================================
FilePlayerPanel::FilePlayerPanel (AudioFilePlayer& player, AudioFormatManager& manager)
    : filePlayer (player), formatManager (manager), transportInfo (player)
{
    //==========================================================================
    // Player panel label:
    addAndMakeVisible (playerPanelLabel);
    playerPanelLabel.setFont (headingFont);
    const auto headingColour
    = getLookAndFeel().findColour (AppLookAndFeel::headingColourId);
    playerPanelLabel.setColour (Label::textColourId, headingColour);
    playerPanelLabel.setText("Audio File Player", dontSendNotification);

    //==========================================================================
    // Set up File management UI components
    addAndMakeVisible (fileButton);
    fileButton.setButtonText ("Open File...");
    fileButton.onClick = [this] { fileButtonClicked(); };

    addAndMakeVisible (currentFileLabel);
    currentFileLabel.setText ("File: <none>", dontSendNotification);

    //==========================================================================
    // Set up transport UI components

    // Play / Pause button:
    addAndMakeVisible (playButton);
    playButton.setButtonText ("Play");
    const auto playColour
    = getLookAndFeel().findColour (AppLookAndFeel::playButtonColourId);
    playButton.setColour (TextButton::buttonColourId, playColour);
    playButton.onClick = [this]
    {
        filePlayer.playPause();
    };

    playButton.setEnabled (false);

    filePlayer.onTransportStarted = [this]
    {
        playButton.setButtonText ("Pause");
        playButton.setEnabled (true);
        stopButton.setEnabled (true);
    };

    filePlayer.onTransportPaused = [this]
    {
        playButton.setButtonText ("Resume");
        playButton.setEnabled (true);
        stopButton.setEnabled (true);
    };

    // Stop button:
    addAndMakeVisible (stopButton);
    stopButton.setButtonText ("Stop");
    const auto stopColour
    = getLookAndFeel().findColour (AppLookAndFeel::stopButtonColourId);
    stopButton.setColour (TextButton::buttonColourId, stopColour);
    stopButton.onClick = [this]
    {
        filePlayer.stop();
    };

    stopButton.setEnabled (false);

    filePlayer.onTransportStopped = [this]
    {
        playButton.setButtonText ("Play");
        playButton.setEnabled (true);
        stopButton.setEnabled (false);
    };

    // Loop button:
    addAndMakeVisible (loopingToggle);
    loopingToggle.setButtonText ("Loop");
    loopingToggle.setToggleState (true, dontSendNotification);
    loopingToggle.onClick = [this]
    {
        filePlayer.setLooping (loopingToggle.getToggleState());
    };

    filePlayer.setLooping (loopingToggle.getToggleState());

    // Transport status:
    addAndMakeVisible (transportInfo);
}

void FilePlayerPanel::resized()
{
    // Manage panel hight
    int requiredHeight = 3 * (buttonHeight + padding) + 2 * padding;
    setSize (getWidth(), requiredHeight);

    // Set control bounds
    auto bounds = getLocalBounds().reduced (padding / 2);

    // Section label:
    playerPanelLabel.setBounds (bounds.removeFromTop (buttonHeight + padding)
                                .withTrimmedTop (padding)
                                .withTrimmedLeft (padding));

    // File management UI components:
    auto fileManagementBounds = bounds.removeFromTop (buttonHeight + padding)
        .withTrimmedTop (padding)
        .withTrimmedLeft (padding);

    auto fileButtonBounds = fileManagementBounds.removeFromLeft (buttonWidth);
    fileButton.setBounds (fileButtonBounds);

    fileManagementBounds.removeFromLeft (padding);   // add spacing

    auto currentFileLabelBounds = fileManagementBounds.withTrimmedRight (padding);
    currentFileLabel.setBounds (currentFileLabelBounds);

    // Transport UI components:
    auto transportButtonsBounds = bounds.removeFromTop (buttonHeight + padding)
        .withTrimmedTop (padding)
        .withTrimmedLeft (padding);

    auto playButtonBounds = transportButtonsBounds.removeFromLeft (buttonWidth);
    playButton.setBounds (playButtonBounds);

    transportButtonsBounds.removeFromLeft (padding);    // add spacing

    auto stopButtonBounds = transportButtonsBounds.removeFromLeft (buttonWidth);
    stopButton.setBounds (stopButtonBounds);

    transportButtonsBounds.removeFromLeft (padding);    // add spacing

    auto loopingToggleBounds = transportButtonsBounds.removeFromLeft (buttonWidth);
    loopingToggle.setBounds (loopingToggleBounds);

    transportButtonsBounds.removeFromRight (padding);   // add spacing

    transportInfo.setBounds (transportButtonsBounds.removeFromRight (buttonWidth));
}

void FilePlayerPanel::fileButtonClicked()
{
    fileChooser
    = std::make_unique<FileChooser> ("Select a file to play...",
                                     File(),
                                     formatManager.getWildcardForAllFormats());

    auto fileChooserFlags = FileBrowserComponent::openMode
                          | FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync (fileChooserFlags, [this] (const FileChooser& fc)
    {
        auto file = fc.getResult(); // retrieve the picked file

        // TODO: Add error warnings

        if (file == File())         // if invalid file, abort
            return;

        // TODO: We're not supporting mp3, add warning (or add support)

        auto* reader = formatManager.createReaderFor (file);

        filePlayer.setAudioFormatReader (reader);

        playButton.setEnabled (true);
        currentFileLabel.setText ("File: " + file.getFileName(), dontSendNotification);
    });
}

//==============================================================================

FilePlayerPanel::TransportStateInfo::TransportStateInfo (const AudioFilePlayer& player)
    : filePlayer (player)
{
    addAndMakeVisible (currentPositionLabel);
    currentPositionLabel.setText ("Stopped", dontSendNotification);
    currentPositionLabel.setJustificationType (Justification::centred);

    startTimer (100);
}

void FilePlayerPanel::TransportStateInfo::paint (Graphics& g)
{
    g.setColour (getLookAndFeel().findColour (Slider::textBoxOutlineColourId));
    g.drawRect (getLocalBounds());
}

void FilePlayerPanel::TransportStateInfo::resized()
{
    currentPositionLabel.setBounds (getLocalBounds());
}

void FilePlayerPanel::TransportStateInfo::timerCallback()
{
    if (! filePlayer.isPlaying())
    {
        currentPositionLabel.setText ("Stopped", dontSendNotification);
        return;
    }

    RelativeTime position (filePlayer.getCurrentPosition());

    auto minutes = static_cast<int>(position.inMinutes());
    auto seconds = static_cast<int>(position.inSeconds()) % 60;

    auto positionString = String::formatted ("%02d:%02d", minutes, seconds);

    currentPositionLabel.setText (positionString, dontSendNotification);
}