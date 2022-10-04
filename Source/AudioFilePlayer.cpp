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
}

//==============================================================================
void AudioFilePlayer::setAudioFormatReader (AudioFormatReader* reader)
{
    if (reader == nullptr)      // if reader is not created, abort
        return;

    // Pass reader ownership to newSource:
    auto newSource = std::make_unique<AudioFormatReaderSource> (reader, true);

    // Set input source for the transportSource object:
    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);

    // Transfer memory ownership of the audio source to readerSource ptr:
    readerSource.reset (newSource.release());

    // Update looping status of the new readerSource:
    readerSource->setLooping (isLooping);

    // Stop transport:
    changeState (TransportState::Stopped);
}

//==========================================================================
void AudioFilePlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void AudioFilePlayer::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
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

void AudioFilePlayer::setLooping (bool shouldLoop)
{
    isLooping = shouldLoop;

    if (readerSource.get() != nullptr)
        readerSource->setLooping (isLooping);
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
