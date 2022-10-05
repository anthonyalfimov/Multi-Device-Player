/*
  ==============================================================================

    FilePlayerPanel.h
    Created: 18 Sep 2022 7:28:15pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioFilePlayer.h"
#include "InterfacePanel.h"

//==============================================================================
class FilePlayerPanel  : public InterfacePanel
{
public:
    FilePlayerPanel (AudioFilePlayer& player, AudioFormatManager& manager);

    //==========================================================================
    void resized() override;

private:
    // Panel label
    Label playerPanelLabel;

    //==========================================================================
    // Audio file management:
    void fileButtonClicked();

    AudioFilePlayer& filePlayer;
    AudioFormatManager& formatManager;

    std::unique_ptr<FileChooser> fileChooser;

    // Audio file management components:
    TextButton fileButton;
    Label currentFileLabel;

    //==========================================================================
    // Transport components:

    class TransportStateInfo  : public Component, public Timer
    {
    public:
        explicit TransportStateInfo (const AudioFilePlayer& audioPlayer);

        //======================================================================
        void paint (Graphics& g) override;
        void resized() override;

        //======================================================================
        void timerCallback() override;

    private:
        const AudioFilePlayer& filePlayer;
        Label currentPositionLabel;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportStateInfo);
    };

    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopingToggle;
    TransportStateInfo transportInfo;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePlayerPanel);
};
