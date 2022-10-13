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

        double nominalSampleRate = 44100.0;
        int blockSize = 32;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PushAudioSource)
    };

    class AudioFifoSource  : public AudioSource
    {
    public:
        explicit AudioFifoSource (AudioFifo& af) : fifo (af) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override {}
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            static double count = 0.0;
            if (fifo.getNumReady() < bufferToFill.numSamples)
                DBG ("Pop skipped #" << (++count));
            fifo.pop (bufferToFill);
        }
        void releaseResources() override {}

    private:
        AudioFifo& fifo;
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

    private:
        MultiDevicePlayer& owner;
        DelayAudioSource delay;
        const double maxLatency;

        double nominalSampleRate = 44100.0;
        int blockSize = 32;
        int popBlockSize = 32;

        AudioFifoSource sharedBufferShource { owner.sharedBuffer };
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
