/*
  ==============================================================================

    MultiDevicePlayer.h
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioFifo.h"
#include "DelayAudioSource.h"

class MultiDevicePlayer
{
public:
    explicit MultiDevicePlayer (double maxLatencyInMs);

    //==========================================================================
    void initialiseAudio (AudioSource* src, int numOutputChannels);
    void shutdownAudio();

    //==========================================================================
    /** [Realtime] [Thread-safe]
        Returns a pointer to the atomic latency compensation value. Latency
        between Main and Linked devices is set in milliseconds.

        If latency parameter is positive, the Main device will be delayed by
        the given amount of milliseconds.

        If latency parameter is negative, the Linked device will be delayed
        by an absolute value of the given time in milliseconds.
    */
    std::atomic<float>* getLatencyParameter() { return &latency; }

    //==========================================================================
    // Device managers
    AudioDeviceManager mainDeviceManager;
    AudioDeviceManager linkedDeviceManager;

private:
    // Latency compensation
    std::atomic<float> latency { 0.0f };    // [ms]
    const double maxLatency;

    //==========================================================================
    // Shared audio buffer facilities
    AudioFifo fifoBuffer;
    SpinLock popMutex;

    //==========================================================================
    // Objects for streaming audio from an audio source to managed devices
    AudioSourcePlayer mainSourcePlayer;
    AudioSourcePlayer linkedSourcePlayer;

    //==========================================================================
    // Audio sources for managed devices
    class MainAudioSource  : public AudioSource
    {
    public:
        MainAudioSource (MultiDevicePlayer& mdp);

        //======================================================================
        void setSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        AudioSource* source = nullptr;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioSource)
    };

    class LinkedAudioSource  : public AudioSource
    {
    public:
        explicit LinkedAudioSource (MultiDevicePlayer& mdp);

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinkedAudioSource)
    };

    friend class MainAudioSource;
    friend class LinkedAudioSource;

    //==========================================================================
    MainAudioSource mainSource;
    LinkedAudioSource linkedSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
