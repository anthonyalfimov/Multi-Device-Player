/*
  ==============================================================================

    AudioFilePlayer.cpp
    Created: 18 Sep 2022 7:27:20pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "AudioFilePlayer.h"

AudioFilePlayer::AudioFilePlayer()
{
    transportSource.addChangeListener (this);

    //==========================================================================
    // Check that atomic bool is lock-free
    static_assert (std::atomic<bool>::is_always_lock_free,
                   "std::atomic for type bool must be always lock free");
}

//==============================================================================
void AudioFilePlayer::setAudioFormatReader (AudioFormatReader* reader)
{
    if (reader == nullptr)      // if reader is not created, abort
        return;

    // Pass reader ownership to newSource:
    auto newSource = std::make_unique<AudioFormatReaderSource> (reader, true);

    {
        SpinLock::ScopedLockType readerLoopingLock (readerLoopingMutex);

        // Set input source for the transportSource object:
        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);

        // Transfer memory ownership of the audio source to readerSource ptr:
        readerSource.reset (newSource.release());

        // Update transport state:
        changeState (TransportState::Stopped);
    }
}

//==========================================================================
void AudioFilePlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioFilePlayer::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    {
        SpinLock::ScopedTryLockType readerLoopingLock (readerLoopingMutex);

        if (readerLoopingLock.isLocked() && readerSource != nullptr)
            readerSource->setLooping (looping.load());
    }

    transportSource.getNextAudioBlock (info);

    if (! transportSource.isPlaying())
    {
        shouldFadeIn = true;
        return;
    }

    if (shouldFadeIn)
    {
        // Just started playing, so fade in the first block:
        const int fadeInLength = jmin (256, info.numSamples);
        info.buffer->applyGainRamp (info.startSample, fadeInLength, 0.0f, 1.0f);
        shouldFadeIn = false;
    }
}

void AudioFilePlayer::releaseResources()
{
    transportSource.releaseResources();
}

//==============================================================================
void AudioFilePlayer::playPause()
{
    if ((state == TransportState::Stopped) || (state == TransportState::Paused))
        changeState (TransportState::Starting);
    else if (state == TransportState::Playing)
        changeState (TransportState::Pausing);
}

void AudioFilePlayer::stop()
{
    if (state == TransportState::Stopped)
        return;

    if (state == TransportState::Paused)
        changeState (TransportState::Stopped);
    else
        changeState (TransportState::Stopping);
}

//==============================================================================
double AudioFilePlayer::getCurrentPosition() const
{
    // getCurrentPosition() method accesses readerSource, so we need to lock:
    SpinLock::ScopedLockType readerLoopingLock (readerLoopingMutex);
    return transportSource.getCurrentPosition();
}

//==============================================================================
void AudioFilePlayer::addChangeListener (ChangeListener* listener)
{
    transportSource.addChangeListener (listener);
}

//==============================================================================
void AudioFilePlayer::changeState (TransportState newState)
{
    if (state == newState)
        return;

    state = newState;

    switch (state)
    {
        case TransportState::Stopped:
            transportSource.setPosition (0.0);

            if (onTransportStopped != nullptr)
                onTransportStopped();

            break;

        case TransportState::Starting:
            transportSource.start();
            break;

        case TransportState::Playing:
            if (onTransportStarted != nullptr)
                onTransportStarted();

            break;

        case TransportState::Pausing:
            transportSource.stop();
            break;

        case TransportState::Paused:
            if (onTransportPaused != nullptr)
                onTransportPaused();

            break;

        case TransportState::Stopping:
            transportSource.stop();
            break;

        default:
            break;
    }
}

//==============================================================================
void AudioFilePlayer::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source != &transportSource)
        return;

    if (transportSource.isPlaying())
        changeState (TransportState::Playing);
    else if ((state == TransportState::Stopping) || (state == TransportState::Playing))
        changeState (TransportState::Stopped);
    else if (state == TransportState::Pausing)
        changeState (TransportState::Paused);
}
