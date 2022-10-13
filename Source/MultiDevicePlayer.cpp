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
    : mainSource (*this, maxLatencyInMs), linkedSource (*this, maxLatencyInMs)
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
void MultiDevicePlayer::resizeSharedBuffer (int numChannels)
{
    const int bufferSize = 4 * jmax (mainSource.getPushBlockSize(),
                                     linkedSource.getPopBlockSize());

    if (numChannels < 0)
        numChannels = sharedBuffer.getNumChannels();

    if (sharedBuffer.getTotalSize() == bufferSize
        && sharedBuffer.getNumChannels() == numChannels)
        return;

    sharedBuffer.setSize (numChannels, bufferSize);
}

//==============================================================================
MultiDevicePlayer::PushAudioSource::
    PushAudioSource (MultiDevicePlayer& mdp, double maxLatencyInMs)
        : owner (mdp), maxLatency (maxLatencyInMs)
{
    
}

void MultiDevicePlayer::PushAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    if (source != nullptr)
        source->prepareToPlay (samplesPerBlockExpected, sampleRate);

    const int numChannels = owner.mainDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    delay.setDelayBufferSize (numChannels, maxLatency);
    delay.prepareToPlay (samplesPerBlockExpected, sampleRate);

    {
        SpinLock::ScopedLockType popLock (owner.popMutex);
        SpinLock::ScopedLockType resizeLock (owner.resizeMutex);

        blockSize = samplesPerBlockExpected;
        nominalSampleRate = sampleRate;

        // NB! Always update the resampling ratio before resizing the shared
        //     buffer because pop block size depends on the ratio.
        owner.linkedSource.updateResamplingRatio();
        owner.resizeSharedBuffer (numChannels);
    }
}

void MultiDevicePlayer::PushAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Process audio and push it to the shared buffer
    if (source != nullptr)
        source->getNextAudioBlock (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();

    owner.sharedBuffer.push (bufferToFill);

    // Delay audio for latency compensation
    const float latencyValue = owner.latency.load();

    if (latencyValue > 0.0)
        delay.setDelayInMs (latencyValue);
    else
        delay.setDelayInMs (0.0f);

    delay.getNextAudioBlock (bufferToFill);
}

void MultiDevicePlayer::PushAudioSource::releaseResources()
{
    if (source != nullptr)
        source->releaseResources();

    delay.releaseResources();
}

//==============================================================================
MultiDevicePlayer::PopAudioSource::
    PopAudioSource (MultiDevicePlayer& mdp, double maxLatencyInMs)
        : owner (mdp), maxLatency (maxLatencyInMs)
{

}

void MultiDevicePlayer::PopAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const int numChannels = owner.linkedDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    delay.setDelayBufferSize (numChannels, maxLatency);
    delay.prepareToPlay (samplesPerBlockExpected, sampleRate);

    {
        SpinLock::ScopedLockType pushLock (owner.pushMutex);
        SpinLock::ScopedLockType resizeLock (owner.resizeMutex);

        nominalSampleRate = sampleRate;
        blockSize = samplesPerBlockExpected;

        resampler = std::make_unique<ResamplingAudioSource> (&sharedBufferShource,
                                                             false,
                                                             numChannels);

        // NB! Always update the resampling ratio before resizing the shared
        //     buffer because pop block size depends on the ratio.
        initialiseResampling();
        owner.resizeSharedBuffer();
    }
}

void MultiDevicePlayer::PopAudioSource::
        getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Pop audio from the shared buffer
    {
        SpinLock::ScopedTryLockType lock (owner.popMutex);

        // TODO: Should we lock inside or outside the AudioSource for the Resampler?
        // If it's outside, we keep all the handling of not getting a lock here,
        // otherwise, we need to put it into the AudioSource object.
        // On the other hand, if we lock here, we lock for the full duration of the
        // ResamplingAudioSource operation.
        // What about underflow and overflow handling? Is there a preference from
        // those?

        if (lock.isLocked())
            resampler->getNextAudioBlock (bufferToFill);
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

void MultiDevicePlayer::PopAudioSource::releaseResources()
{
    delay.releaseResources();

    {
        SpinLock::ScopedLockType resizeLock (owner.resizeMutex);
        resampler->releaseResources();
        resampler.reset();
    }
}

void MultiDevicePlayer::PopAudioSource::updateResamplingRatio()
{
    if (resampler == nullptr)
        return;

    resampler->releaseResources();
    initialiseResampling();
}

void MultiDevicePlayer::PopAudioSource::initialiseResampling()
{
    double resamplingRatio = owner.mainSource.getSampleRate() / nominalSampleRate;

    // TODO: Should we use more accurate conversion or pad the buffer size?
    popBlockSize = roundToInt (blockSize * resamplingRatio);

    // TODO: ResamplingAudioSource can reallocate during processing
    // 1. Set resamplingRatio that would allocate buffer with excess
    // 2. Call prepareToPlay to trigger buffer allocation
    // 3. Set resamplingRatio to the actual value
    
    resampler->setResamplingRatio (resamplingRatio);
    resampler->prepareToPlay (blockSize, nominalSampleRate);
}
