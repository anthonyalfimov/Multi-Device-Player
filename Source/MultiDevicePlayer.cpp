/*
  ==============================================================================

    MultiDevicePlayer.cpp
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "MultiDevicePlayer.h"

// TODO: Consider handling input audio

MultiDevicePlayer::MultiDevicePlayer (double maxLatencyInMs)
    : maxLatency (maxLatencyInMs), mainSource (*this), linkedSource (*this)
{
    //==========================================================================
    // Check that atomic float is lock-free
    static_assert (std::atomic<float>::is_always_lock_free,
                   "std::atomic for type float must be always lock free");
}

//==============================================================================
void MultiDevicePlayer::initialiseAudio (AudioSource* src, int numOutputChannels)
{
    mainSource.setSource (src);

    mainDeviceManager.initialiseWithDefaultDevices (0, numOutputChannels);
    linkedDeviceManager.initialiseWithDefaultDevices (0, numOutputChannels);

    mainDeviceManager.addAudioCallback (&mainSourcePlayer);
    linkedDeviceManager.addAudioCallback (&linkedSourcePlayer);

    mainSourcePlayer.setSource (&mainSource);
    linkedSourcePlayer.setSource (&linkedSource);
}

void MultiDevicePlayer::shutdownAudio()
{
    mainSourcePlayer.setSource (nullptr);
    linkedSourcePlayer.setSource (nullptr);

    mainDeviceManager.removeAudioCallback (&mainSourcePlayer);
    linkedDeviceManager.removeAudioCallback (&linkedSourcePlayer);

    mainDeviceManager.closeAudioDevice();
    linkedDeviceManager.closeAudioDevice();

    mainSource.setSource (nullptr);
}

//==============================================================================
MultiDevicePlayer::MainAudioSource::MainAudioSource (MultiDevicePlayer& mdp)
    : owner (mdp)
{
    
}

void MultiDevicePlayer::MainAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    if (source != nullptr)
        source->prepareToPlay (samplesPerBlockExpected, sampleRate);

    const int numChannels = owner.mainDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    delay.setDelayBufferSize (numChannels, owner.maxLatency);
    delay.prepareToPlay (samplesPerBlockExpected, sampleRate);

    SpinLock::ScopedLockType lock (owner.popMutex);
    owner.fifoBuffer.setSize (numChannels, samplesPerBlockExpected * 4);
}

void MultiDevicePlayer::MainAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Process audio and push it to the shared buffer
    if (source != nullptr)
        source->getNextAudioBlock (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();

    owner.fifoBuffer.push (bufferToFill);

    // Delay audio for latency compensation
    const float latencyValue = owner.latency.load();

    if (latencyValue > 0.0)
        delay.setDelayInMs (latencyValue);
    else
        delay.setDelayInMs (0.0f);

    delay.getNextAudioBlock (bufferToFill);
}

void MultiDevicePlayer::MainAudioSource::releaseResources()
{
    if (source != nullptr)
        source->releaseResources();

    delay.releaseResources();
}

//==============================================================================
MultiDevicePlayer::LinkedAudioSource::LinkedAudioSource (MultiDevicePlayer& mdp)
    : owner (mdp)
{

}

void MultiDevicePlayer::LinkedAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const int numChannels = owner.linkedDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    delay.setDelayBufferSize (numChannels, owner.maxLatency);
    delay.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MultiDevicePlayer::LinkedAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Pop audio from the shared buffer
    {
        SpinLock::ScopedTryLockType lock (owner.popMutex);

        if (lock.isLocked())
            owner.fifoBuffer.pop (bufferToFill);
        else
            bufferToFill.clearActiveBufferRegion();
    }

    // Delay audio for latency compensation
    const float latencyValue = owner.latency.load();

    if (latencyValue < 0.0)
        delay.setDelayInMs (-latencyValue);
    else
        delay.setDelayInMs (0.0f);

    delay.getNextAudioBlock (bufferToFill);
}

void MultiDevicePlayer::LinkedAudioSource::releaseResources()
{
    delay.releaseResources();
}

