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
MainComponent::MainComponent() : audioOutput (maxLatencyInMs)
{
    //==========================================================================
    // Update Look And Feel
    LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
    setLookAndFeel (&lookAndFeel);

    //==========================================================================
    // Set up audio playback
    audioOutput.initialiseAudio (this, 2);

    //==========================================================================
    // Set up file player
    filePlayerPanel = std::make_unique<FilePlayerPanel> (filePlayer, formatManager);
    addAndMakeVisible (filePlayerPanel.get());

    //==========================================================================
    // Set up device panel
    devicePanel = std::make_unique<DevicePanel> (audioOutput.mainDeviceManager,
                                                 audioOutput.linkedDeviceManager,
                                                 syncPlayer, maxLatencyInMs);
    devicePanel->attachLatencyParameter (audioOutput.getLatencyParameter());
    addAndMakeVisible (devicePanel.get());

    //==========================================================================
    // Set initial component size
    setSize (600, 600);
    resized();

    //==========================================================================
    // Set up transport management facilities
    formatManager.registerBasicFormats();

    filePlayer.addChangeListener (&deviceSelectorUpdater);
    syncPlayer.addChangeListener (&deviceSelectorUpdater);

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
    //==========================================================================
    // Shutdown audio
    audioOutput.shutdownAudio();

    //==========================================================================
    // Release Look And Feel
    LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    syncPlayer.prepareToPlay (samplesPerBlockExpected, sampleRate);
    filePlayer.prepareToPlay (samplesPerBlockExpected, sampleRate);

    const auto numChannels = audioOutput.mainDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();

    crossfadeBuffer.setSize (numChannels, samplesPerBlockExpected, false, true);
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

    if (syncPlayer.isPlaying())
    {
        shouldFadeFromSync = true;
        syncPlayer.getNextAudioBlock (bufferToFill);

        if (shouldFadeToSync)
        {
            fadeAudioSource (bufferToFill, filePlayer, false);
            shouldFadeToSync = false;
        }
    }
    else
    {
        shouldFadeToSync = true;

        if (shouldFadeFromSync)
        {
            syncPlayer.getNextAudioBlock (bufferToFill);
            fadeAudioSource (bufferToFill, filePlayer, true);
            shouldFadeFromSync = false;
        }
        else
        {
            filePlayer.getNextAudioBlock (bufferToFill);
        }
    }
}

void MainComponent::releaseResources()
{
    syncPlayer.releaseResources();
    filePlayer.releaseResources();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // Fill background
    const auto bgColour = getLookAndFeel()
                                .findColour (ResizableWindow::backgroundColourId);
    g.fillAll (bgColour);

    // Paint File Player Panel background
    g.setColour (bgColour.darker (0.2f));
    const auto filePlayerPanelBounds = filePlayerPanel->getBounds().toFloat();
    g.drawRoundedRectangle (filePlayerPanelBounds,
                            InterfacePanel::corner,
                            InterfacePanel::line);

    // Paint Device Panel background
    g.setColour (bgColour.darker (0.2f));
    const auto devicePanelBounds = devicePanel->getBounds().toFloat();
    g.fillRoundedRectangle (devicePanelBounds,
                            InterfacePanel::corner);
    g.drawRoundedRectangle (devicePanelBounds,
                            InterfacePanel::corner,
                            InterfacePanel::line);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced (InterfacePanel::padding);

    //==========================================================================
    // File Player Panel:
    filePlayerPanel->setBounds (bounds.removeFromTop (filePlayerPanel->getHeight()));
    bounds.removeFromTop (InterfacePanel::padding);     // add spacing

    //==========================================================================
    // Device Panel:
    devicePanel->setBounds (bounds);
}

//==============================================================================
void MainComponent::fadeAudioSource (const AudioSourceChannelInfo& bufferToFill,
                                     AudioSource& sourceToFade,
                                     bool shouldFadeIn)
{
    AudioSourceChannelInfo crossfadeInfo (&crossfadeBuffer,
                                          bufferToFill.startSample,
                                          bufferToFill.numSamples);
    sourceToFade.getNextAudioBlock (crossfadeInfo);

    const int fadeLength = jmin (256, bufferToFill.numSamples);

    if (shouldFadeIn)
    {
        // Fade buffer in
        crossfadeBuffer.applyGainRamp (bufferToFill.startSample, fadeLength, 0.0f, 1.0f);
    }
    else
    {
        // Fade buffer out
        crossfadeBuffer.applyGainRamp (bufferToFill.startSample, fadeLength, 1.0f, 0.0f);

        if (bufferToFill.numSamples > 256)
        {
            crossfadeBuffer.clear (bufferToFill.startSample + 256,
                                   bufferToFill.numSamples - 256);
        }
    }

    const int numChannels = jmin (crossfadeBuffer.getNumChannels(),
                                  bufferToFill.buffer->getNumChannels());

    for (int ch = 0; ch < numChannels; ++ch)
    {
        bufferToFill.buffer->addFrom (ch,
                                      bufferToFill.startSample,
                                      crossfadeBuffer,
                                      ch,
                                      bufferToFill.startSample,
                                      bufferToFill.numSamples);
    }
}

//==============================================================================
void MainComponent::DeviceSelectorUpdater::
        changeListenerCallback (ChangeBroadcaster* source)
{
    if (owner->filePlayer.isPlaying() || owner->syncPlayer.isPlaying())
        owner->devicePanel->setDeviceSelectorEnabled (false);
    else
        owner->devicePanel->setDeviceSelectorEnabled (true);
}
