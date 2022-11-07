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
#include "AudioFifoSource.h"
#include "DelayAudioSource.h"

class MultiDevicePlayer  : private Timer
{
public:
    explicit MultiDevicePlayer (double maxLatencyInMs);

    //==========================================================================
    void initialiseAudio (AudioSource* src, int numOutputChannels);
    void shutdownAudio();

    //==========================================================================
    /** [Realtime] [Thread-safe]
     Sets the atomic latency compensation value. Latency between Main and Linked
     devices is defined in milliseconds.

     If latency parameter is positive, the Main device will be delayed by
     the given amount of milliseconds.

     If latency parameter is negative, the Linked device will be delayed
     by an absolute value of the given time in milliseconds.
    */
    void setLatency (float newLatencyInMs) { latency.store (newLatencyInMs); }

    /** [Realtime] [Thread-safe]
     Sets main device playback gain atomic value.
    */
    void setMainGain (float newGain) { mainSourcePlayer.setGain (newGain); }

    /** [Realtime] [Thread-safe]
     Sets linked device playback gain atomic value.
    */
    void setLinkedGain (float newGain) { linkedSourcePlayer.setGain (newGain); }

    //==========================================================================
    // Device managers
    AudioDeviceManager mainDeviceManager;
    AudioDeviceManager linkedDeviceManager;

private:
    // Latency compensation
    std::atomic<float> latency { 0.0f };    // [ms]

    //==========================================================================
    // Shared audio buffer facilities
    AudioFifo sharedBuffer;
    SpinLock popMutex;
    SpinLock pushMutex;
    SpinLock resizeMutex;

    /** [Non-realtime] [Non-thread-safe]
        Checks whether sharedBuffer size needs to be changed and resizes it
        if necessary.

        @param numChannels  pass the new number of channels required,
                            or -1 to keep the channel count unchanged.
    */
    void resizeSharedBuffer (int numChannels = -1);

    //==========================================================================
    // Audio device management
    /** AudioDeviceManager is not always notified when the device sample rate
        is changed. When a discrepancy like this is detected, this function
        should be used to update AudioDeviceManager to the factual sample rate.
    */
    void resetAudioDevice (AudioDeviceManager& manager);
    void timerCallback() override;

    //==========================================================================
    // Objects for streaming audio from an audio source to managed devices
    AudioSourcePlayer mainSourcePlayer;
    AudioSourcePlayer linkedSourcePlayer;

    //==========================================================================
    // Audio sources for managed devices
    class PushAudioSource  : public AudioSource
    {
    public:
        PushAudioSource (MultiDevicePlayer& mdp, double maxLatencyInMs);

        //======================================================================
        /** [Realtime] [Non-tread-safe]
            ...
        */
        void setSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        //======================================================================
        double getSampleRate() const { return nominalSampleRate; }
        int getPushBlockSize() const { return blockSize; }

        //======================================================================
        /** [Non-realtime] [Non-tread-safe]
            Initialise the internal delay buffer for latency compensation.

            Make sure this method is called only after the members
            `blockSize`, `numChannels`, and `nominalSampleRate` have been set.
        */
        void prepareLatencyCompensation (int sharedBufferSize);

        /** Atomic flag that is set when the actual device settings do not
            match its AudioDeviceManager settings
        */
        std::atomic<bool> needsAudioDeviceReset = false;

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        const double maxLatencyDelayInMs;

        /*  Linked device will wait until half of the shared buffer is filled
            before starting playback. Therefore, main device will be ahead of
            the linked device by half the shared buffer size on average.
            The `fixedDelay` value is added to compensate for this latency.
        */
        int fixedDelay = 0;

        //======================================================================
        AudioSource* source = nullptr;

        bool waitForBufferSpace = false;

        //======================================================================
        int numChannels = 2;
        double nominalSampleRate = 44100.0;
        int blockSize = 32;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PushAudioSource)
    };

    class PopAudioSource  : public AudioSource
    {
    public:
        PopAudioSource (MultiDevicePlayer& mdp, double maxLatencyInMs);

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        //======================================================================
        double getSampleRate() const { return nominalSampleRate; }
        int getPopBlockSize() const { return popBlockSize; }

        //======================================================================
        /** [Non-realtime] [Non-tread-safe]
            Recalculate and set the resampling ratio.
        */
        void updateResamplingRatio();

        /** [Realtime] [Non-tread-safe]
            Indicated that popping from the shared buffer should be halted
            until the buffer is half-filled.
        */
        void haltUntilBufferIsHalfFilled() { waitForBufferToFill = true; }

        /** Atomic flag that is set when the actual device settings do not
            match its AudioDeviceManager settings
        */
        std::atomic<bool> needsAudioDeviceReset = false;

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        const double maxLatencyDelayInMs;

        //======================================================================
        bool waitForBufferToFill = true;

        //======================================================================
        double nominalSampleRate = 44100.0;
        int blockSize = 32;
        int popBlockSize = 32;

        //======================================================================
        AudioFifoSource sharedBufferSource { owner.sharedBuffer };
        std::unique_ptr<ResamplingAudioSource> resampler;

        void initialiseResampling();

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopAudioSource)
    };

    friend class PushAudioSource;
    friend class PopAudioSource;

    //==========================================================================
    PushAudioSource mainSource;
    PopAudioSource linkedSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
