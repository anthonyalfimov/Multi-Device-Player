/*
  ==============================================================================

     Multi-Device Player - A simple cross-platform aggregate device player
     Copyright (C) 2022  Anthony Alfimov

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <https://www.gnu.org/licenses/>.

      ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "AudioFilePlayer.h"
#include "MultiDevicePlayer.h"
#include "InterfacePanel.h"
#include "FilePlayerPanel.h"
#include "DevicePanel.h"
#include "AppLookAndFeel.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public Component,
                       public AudioSource
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    //==========================================================================
    // Transport management
    AudioFilePlayer syncPlayer;
    AudioFilePlayer filePlayer;

    AudioBuffer<float> crossfadeBuffer;

    bool shouldFadeToSync = false;
    bool shouldFadeFromSync = false;

    void fadeAudioSource (const AudioSourceChannelInfo& bufferToFill,
                          AudioSource& sourceToFade,
                          bool shouldFadeIn);

    //==========================================================================
    // Audio Processing
    AudioFormatManager formatManager;
    MultiDevicePlayer audioOutput;

    //==========================================================================
    // Audio parameters
    inline static constexpr double maxLatencyInMs = 250.0 /*ms*/;

    //==========================================================================
    // UI Panels
    std::unique_ptr<FilePlayerPanel> filePlayerPanel;
    std::unique_ptr<DevicePanel> devicePanel;

    //==========================================================================
    // Updating Device Selector state
    class DeviceSelectorUpdater  : public ChangeListener
    {
    public:
        DeviceSelectorUpdater (MainComponent* mc) : owner (mc) {}
        void changeListenerCallback (ChangeBroadcaster* source);

    private:
        MainComponent* owner;
    };

    DeviceSelectorUpdater deviceSelectorUpdater { this };

    //==========================================================================
    // UI Style
    AppLookAndFeel lookAndFeel;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
