/*
  ==============================================================================

    MultiDevicePlayer.cpp
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "MultiDevicePlayer.h"

MultiDevicePlayer::MultiDevicePlayer()
    : mainSource (fifoBuffer), clientSource (fifoBuffer)
{
    mainDeviceManager.initialiseWithDefaultDevices (0, 2);
    clientDeviceManager.initialiseWithDefaultDevices (0, 2);

    mainDeviceManager.addAudioCallback (&mainSourcePlayer);
    clientDeviceManager.addAudioCallback (&clientSourcePlayer);

    mainSourcePlayer.setSource (&mainSource);
    clientSourcePlayer.setSource (&clientSource);
}

MultiDevicePlayer::~MultiDevicePlayer()
{
    mainSourcePlayer.setSource (nullptr);
    clientSourcePlayer.setSource (nullptr);

    mainDeviceManager.removeAudioCallback (&mainSourcePlayer);
    clientDeviceManager.removeAudioCallback (&clientSourcePlayer);

    mainDeviceManager.closeAudioDevice();
    clientDeviceManager.closeAudioDevice();
}

