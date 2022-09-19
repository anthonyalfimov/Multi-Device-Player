/*
  ==============================================================================

    MultiDevicePlayer.cpp
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "MultiDevicePlayer.h"

MultiDevicePlayer::MultiDevicePlayer()
    : mainSource (sharedBuffer), linkedSource (sharedBuffer)
{
    mainDeviceManager.initialiseWithDefaultDevices (0, 2);
    linkedDeviceManager.initialiseWithDefaultDevices (0, 2);

    mainDeviceManager.addAudioCallback (&mainSourcePlayer);
    linkedDeviceManager.addAudioCallback (&linkedSourcePlayer);

    mainSourcePlayer.setSource (&mainSource);
    linkedSourcePlayer.setSource (&linkedSource);
}

MultiDevicePlayer::~MultiDevicePlayer()
{
    mainSourcePlayer.setSource (nullptr);
    linkedSourcePlayer.setSource (nullptr);

    mainDeviceManager.removeAudioCallback (&mainSourcePlayer);
    linkedDeviceManager.removeAudioCallback (&linkedSourcePlayer);

    mainDeviceManager.closeAudioDevice();
    linkedDeviceManager.closeAudioDevice();
}

