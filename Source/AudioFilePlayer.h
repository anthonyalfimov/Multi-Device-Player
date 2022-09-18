/*
  ==============================================================================

    AudioFilePlayer.h
    Created: 18 Sep 2022 7:27:20pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class AudioFilePlayer  : public ChangeListener
{
public:
    AudioFilePlayer();

    //==========================================================================
    // Load audio to play
    void setAudioFormatReader (AudioFormatReader* reader);

    //==========================================================================
    // Audio processing
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);

    void releaseResources();

    void getNextAudioBlock (const AudioSourceChannelInfo& info);

    //==========================================================================
    // Player transport controls
    void playPause();
    void stop();
    void setLooping (bool shouldLoop);

    //==========================================================================
    // Player transport state
    bool readyToPlay() const { return (readerSource != nullptr); }

    enum class TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    TransportState getTransportState() const { return state; }
    double getCurrentPosition() const { return transportSource.getCurrentPosition(); }

    std::function<void()> onTransportStarted;
    std::function<void()> onTransportPaused;
    std::function<void()> onTransportStopped;

    //==========================================================================
    // Change listener callback
    void changeListenerCallback (ChangeBroadcaster* source) override;

private:
    void changeState (TransportState newState);

    bool isLooping = false;

    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state = TransportState::Stopped;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayer);
};
