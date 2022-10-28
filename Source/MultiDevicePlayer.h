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

class MultiDevicePlayer
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
        void setSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        //======================================================================
        double getSampleRate() const { return nominalSampleRate; }
        int getPushBlockSize() const { return blockSize; }

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        const double maxLatency;
        AudioSource* source = nullptr;

        bool waitForBufferSpace = false;

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

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        const double maxLatency;

        bool waitForBufferToFill = true;

        double nominalSampleRate = 44100.0;
        int blockSize = 32;
        int popBlockSize = 32;

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
