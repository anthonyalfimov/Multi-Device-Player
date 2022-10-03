/*
  ==============================================================================

    MultiDevicePlayer.cpp
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "MultiDevicePlayer.h"

MultiDevicePlayer::MultiDevicePlayer()
    : mainSource (*this), clientSource (*this)
{

}

//==============================================================================
void MultiDevicePlayer::initialiseAudio (int numOutputChannels)
{
    mainDeviceManager.initialiseWithDefaultDevices (0, numOutputChannels);
    clientDeviceManager.initialiseWithDefaultDevices (0, numOutputChannels);

    mainDeviceManager.addAudioCallback (&mainSourcePlayer);
    clientDeviceManager.addAudioCallback (&clientSourcePlayer);

    mainSourcePlayer.setSource (&mainSource);
    clientSourcePlayer.setSource (&clientSource);
}

void MultiDevicePlayer::shutdownAudio()
{
    mainSourcePlayer.setSource (nullptr);
    clientSourcePlayer.setSource (nullptr);

    mainDeviceManager.removeAudioCallback (&mainSourcePlayer);
    clientDeviceManager.removeAudioCallback (&clientSourcePlayer);

    mainDeviceManager.closeAudioDevice();
    clientDeviceManager.closeAudioDevice();
}

//==============================================================================
void MultiDevicePlayer::MainAudioSource
        ::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    if (source != nullptr)
        source->prepareToPlay (samplesPerBlockExpected, sampleRate);

    // TODO:  Determine the number of channels based on device configuration
    // TODO:  Consider using a function for buffer size?
    SpinLock::ScopedLockType lock (owner.popMutex);
    owner.fifoBuffer.setSize (2, samplesPerBlockExpected * 4);
}

void MultiDevicePlayer::MainAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (source != nullptr)
        source->getNextAudioBlock (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();

    owner.fifoBuffer.push (bufferToFill);
}

void MultiDevicePlayer::MainAudioSource::releaseResources()
{
    if (source != nullptr)
        source->releaseResources();

    //sharedBuffer.reset();
}

//==============================================================================
void MultiDevicePlayer::ClientAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{

}

void MultiDevicePlayer::ClientAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    SpinLock::ScopedTryLockType lock (owner.popMutex);

    if (lock.isLocked())
        owner.fifoBuffer.pop (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();
}

void MultiDevicePlayer::ClientAudioSource::releaseResources()
{

}

