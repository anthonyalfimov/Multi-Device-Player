/*
  ==============================================================================

    AudioFilePlayer.h
    Created: 18 Sep 2022 7:27:20pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// TODO: Pre-buffer the file that is playing?
// TODO: Examine thread-safety of this class (use thread sanitiser?)

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
    void getNextAudioBlock (const AudioSourceChannelInfo& info);
    void releaseResources();

    //==========================================================================
    // Player transport controls
    void playPause();
    void stop();
    void setLooping (bool shouldLoop);

    //==========================================================================
    // Player transport state
    bool readyToPlay() const { return (readerSource != nullptr); }
    bool isPlaying() const { return transportSource.isPlaying(); }
    bool isLooping() const { return transportSource.isLooping(); }
    double getCurrentPosition() const { return transportSource.getCurrentPosition(); }

    //==========================================================================
    // Transport state change callbacks
    std::function<void()> onTransportStarted;
    std::function<void()> onTransportPaused;
    std::function<void()> onTransportStopped;

    //==========================================================================
    // Register a listener to receive change callbacks
    void addChangeListener (ChangeListener* listener);

private:
    enum class TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    void changeState (TransportState newState);

    bool looping = false;
    bool shouldFadeIn = true;

    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state = TransportState::Stopped;

    //==========================================================================
    // Change listener callback
    void changeListenerCallback (ChangeBroadcaster* source) override;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayer);
};
