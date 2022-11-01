/*
  ==============================================================================

    MultiDevicePlayer.cpp
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "MultiDevicePlayer.h"

MultiDevicePlayer::MultiDevicePlayer (double maxLatencyInMs)
    : mainSource (*this, maxLatencyInMs), linkedSource (*this, maxLatencyInMs)
{
    // Start checking if audio devices need to be reset:
    startTimerHz (10);

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
    const int bufferSize = 6 * jmax (mainSource.getPushBlockSize(),
                                     linkedSource.getPopBlockSize());

    if (numChannels < 0)
        numChannels = sharedBuffer.getNumChannels();

    if (sharedBuffer.getTotalSize() == bufferSize
        && sharedBuffer.getNumChannels() == numChannels)
        return;

    sharedBuffer.setSize (numChannels, bufferSize);
    linkedSource.haltUntilBufferIsHalfFilled(); // start popping only after the buffer
                                                // is sufficiently filled
}

//==============================================================================
void MultiDevicePlayer::resetAudioDevice (AudioDeviceManager& manager)
{
    auto setup = manager.getAudioDeviceSetup();
    setup.sampleRate = manager.getCurrentAudioDevice()->getCurrentSampleRate();
    manager.setAudioDeviceSetup (setup, true);
}

void MultiDevicePlayer::timerCallback()
{
    if (mainSource.needsAudioDeviceReset.load())
        resetAudioDevice (mainDeviceManager);

    if (linkedSource.needsAudioDeviceReset.load())
        resetAudioDevice (linkedDeviceManager);
}

//==============================================================================
MultiDevicePlayer::PushAudioSource::
    PushAudioSource (MultiDevicePlayer& mdp, double maxLatencyInMs)
        : owner (mdp), maxLatency (maxLatencyInMs)
{
    //==========================================================================
    // Check that atomic float is lock-free
    static_assert (std::atomic<bool>::is_always_lock_free,
                   "std::atomic for type bool must be always lock free");
}

void MultiDevicePlayer::PushAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    needsAudioDeviceReset.store (false);

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
    // Check that device sample rate hasn't been externally changed
    if (owner.mainDeviceManager.getCurrentAudioDevice()->getCurrentSampleRate()
        != nominalSampleRate)
    {
        bufferToFill.clearActiveBufferRegion();
        needsAudioDeviceReset.store (true);
        DBG ("Main device: sample rate mismatch");
        return;
    }

    // Process audio and push it to the shared buffer
    if (source != nullptr)
        source->getNextAudioBlock (bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();

    // Push audio to the shared buffer
    {
        SpinLock::ScopedTryLockType pushLock (owner.pushMutex);

        if (pushLock.isLocked())
        {
            const int freeSpace = owner.sharedBuffer.getFreeSpace();
            const int sharedBufferSize = owner.sharedBuffer.getTotalSize();
            const int minFreeSpace = static_cast<int> (1.2f * blockSize);

            if (waitForBufferSpace)
            {
                if (freeSpace >= sharedBufferSize / 2)
                {
                    // Push and fade in
                    owner.sharedBuffer.pushWithRamp (bufferToFill, 0.0f, 1.0f);
                    waitForBufferSpace = false;
                }
            }
            else
            {
                if (freeSpace >= minFreeSpace)
                {
                    // Push
                    owner.sharedBuffer.push (bufferToFill);
                }
                else
                {
                    // Push and fade out
                    owner.sharedBuffer.pushWithRamp (bufferToFill, 1.0f, 0.0f);
                    waitForBufferSpace = true;
                }
            }
        }
    }

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
    //==========================================================================
    // Check that atomic float is lock-free
    static_assert (std::atomic<bool>::is_always_lock_free,
                   "std::atomic for type bool must be always lock free");
}

void MultiDevicePlayer::PopAudioSource::
        prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    needsAudioDeviceReset.store (false);

    const int numChannels = owner.linkedDeviceManager
        .getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();
    delay.setDelayBufferSize (numChannels, maxLatency);
    delay.prepareToPlay (samplesPerBlockExpected, sampleRate);

    {
        SpinLock::ScopedLockType pushLock (owner.pushMutex);
        SpinLock::ScopedLockType resizeLock (owner.resizeMutex);

        nominalSampleRate = sampleRate;
        blockSize = samplesPerBlockExpected;

        resampler = std::make_unique<ResamplingAudioSource> (&sharedBufferSource,
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
    // Check that device sample rate hasn't been externally changed
    if (owner.linkedDeviceManager.getCurrentAudioDevice()->getCurrentSampleRate()
        != nominalSampleRate)
    {
        bufferToFill.clearActiveBufferRegion();
        needsAudioDeviceReset.store (true);
        DBG ("Linked device: sample rate mismatch");
        return;
    }

    // Pop audio from the shared buffer
    {
        SpinLock::ScopedTryLockType popLock (owner.popMutex);

        if (popLock.isLocked())
        {
            const int numReady = owner.sharedBuffer.getNumReady();
            const int sharedBufferSize = owner.sharedBuffer.getTotalSize();
            const int minNumReady = static_cast<int> (1.2f * popBlockSize);

            if (waitForBufferToFill)
            {
                if (numReady >= sharedBufferSize / 2)
                {
                    // Pop and fade in
                    sharedBufferSource.setGainRamp (0.0f, 1.0f);
                    resampler->getNextAudioBlock (bufferToFill);
                    waitForBufferToFill = false;
                }
                else
                {
                    // Clear buffer
                    bufferToFill.clearActiveBufferRegion();
                }
            }
            else
            {
                if (numReady >= minNumReady)
                {
                    // Pop
                    sharedBufferSource.setGainRamp (1.0f, 1.0f);
                    resampler->getNextAudioBlock (bufferToFill);
                }
                else
                {
                    // Pop and fade out
                    sharedBufferSource.setGainRamp (1.0f, 0.0f);
                    resampler->getNextAudioBlock (bufferToFill);
                    waitForBufferToFill = true;
                }
            }
        }
        else
        {
            // Clear buffer
            bufferToFill.clearActiveBufferRegion();
        }
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

        if (resampler != nullptr)
        {
            resampler->releaseResources();
            resampler.reset();
        }
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

    // Max block size that can be requested by ResamplingAudioSource:
    popBlockSize = roundToInt (blockSize * resamplingRatio) + 3;

    // TODO: ResamplingAudioSource can reallocate during processing
    // 1. Set resamplingRatio that would allocate buffer with excess
    // 2. Call prepareToPlay to trigger buffer allocation
    // 3. Set resamplingRatio to the actual value
    
    resampler->setResamplingRatio (resamplingRatio);
    resampler->prepareToPlay (blockSize, nominalSampleRate);
}
