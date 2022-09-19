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

#include "MainComponent.h"
#include "InterfacePanel.h"

//==============================================================================
MainComponent::MainComponent()
    : devicePanel (audioOutput.mainDeviceManager, audioOutput.linkedDeviceManager),
      controlPanel (syncPlayer, filePlayer, formatManager, maxLatencyInMs)
{
    //==========================================================================
    // Update Look And Feel
    auto bgColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    getLookAndFeel().setColour (ScrollBar::thumbColourId, bgColour.brighter());

    //==========================================================================
    // Set up audio playback
    audioOutput.setAudioSource (this);

    // MARK: Trigger PrepareToPlay to prepare `this` AudioSource
    // TODO: Consider initialising devices from outside MultiDevicePlayer
    //       e.g. add initialisation method that can be called 
    audioOutput.mainDeviceManager.initialiseWithDefaultDevices (0, 2);

    //==========================================================================
    // Set up device panel
    addAndMakeVisible (devicePanelViewport);
    devicePanelViewport.setViewedComponent (&devicePanel, false);
    devicePanelViewport.setScrollBarsShown (true, false);

    //==========================================================================
    // Set up control panel
    addAndMakeVisible (controlPanelViewport);
    controlPanelViewport.setViewedComponent (&controlPanel, false);
    controlPanelViewport.setScrollBarsShown (true, false);

    //==========================================================================
    // Set initial component size
    setSize (1200, 400);
    devicePanel.resized();
    controlPanel.resized();
    resized();

    //==========================================================================
    // Set up transport management facilities
    formatManager.registerBasicFormats();

    //==========================================================================
    // Set up sync track
    syncPlayer.setAudioFormatReader (formatManager.createReaderFor
        (std::make_unique<MemoryInputStream> (BinaryData::SyncTrack_wav,
                                              BinaryData::SyncTrack_wavSize,
                                              false)));
    syncPlayer.setLooping (true);
}

MainComponent::~MainComponent()
{

}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    syncPlayer.prepareToPlay (samplesPerBlockExpected, sampleRate);
    filePlayer.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    /* Denormals are temporarily disabled when this object is created at the
       beginning of the process block and re-enabled when it's destroyed at the
       end of the process block. Therefore, anything that happens within the
       process block doesn't need to disable denormals - they won't be
       re-enabled until the end of the process block.
    */
    ScopedNoDenormals noDenormals;

    if (filePlayer.readyToPlay())
        filePlayer.getNextAudioBlock (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    syncPlayer.releaseResources();
    filePlayer.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    const auto padding = InterfacePanel::padding;
    auto bounds = getLocalBounds().reduced (padding / 2);

    //==========================================================================
    // Device Panel:
    auto devicePanelBounds = bounds.removeFromLeft (600);
    devicePanelViewport.setBounds (devicePanelBounds);
    devicePanel.setSize (devicePanelBounds.getWidth(), devicePanel.getHeight());

    if (devicePanelViewport.getViewHeight() < devicePanel.getHeight())
        devicePanelViewport.setBounds (devicePanelBounds.withTrimmedRight (-3));

    //==========================================================================
    // Control Panel:
    controlPanelViewport.setBounds (bounds);
    controlPanel.setSize (bounds.getWidth(), controlPanel.getHeight());

    if (controlPanelViewport.getViewHeight() < controlPanel.getHeight())
        controlPanelViewport.setBounds (bounds.withTrimmedRight (-3));
}
