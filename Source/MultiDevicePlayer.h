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

class MultiDevicePlayer
{
public:
    MultiDevicePlayer();
    ~MultiDevicePlayer();

    void setSource (AudioSource* src) { mainSource.setSource (src); }

    AudioDeviceManager mainDeviceManager;
    AudioDeviceManager clientDeviceManager;

private:
    AudioSourcePlayer mainSourcePlayer;
    AudioSourcePlayer clientSourcePlayer;

    AudioFifo fifoBuffer;

    //==========================================================================
    class MainAudioSource  : public AudioSource
    {
    public:
        explicit MainAudioSource (AudioFifo& sb) : sharedBuffer (sb) {}

        //======================================================================
        void setSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {
            if (source != nullptr)
                source->prepareToPlay (samplesPerBlockExpected, sampleRate);

            // TODO:  Determine the number of channels based on device configuration
            // TODO:  Consider using a function for buffer size?
            // FIXME: This operation is not thread-safe!
            sharedBuffer.setSize (2, samplesPerBlockExpected * 4);
        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            if (source != nullptr)
                source->getNextAudioBlock (bufferToFill);
            else
                bufferToFill.clearActiveBufferRegion();

            sharedBuffer.push (bufferToFill);
        }

        void releaseResources() override
        {
            if (source != nullptr)
                source->releaseResources();

            //sharedBuffer.reset();
        }
    private:
        AudioSource* source = nullptr;
        AudioFifo& sharedBuffer;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioSource)
    };

    class ClientAudioSource  : public AudioSource
    {
    public:
        explicit ClientAudioSource (AudioFifo& sb) : sharedBuffer (sb) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {

        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            sharedBuffer.pop (bufferToFill);
        }

        void releaseResources() override
        {

        }

    private:
        AudioFifo& sharedBuffer;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClientAudioSource)
    };

    //==========================================================================
    MainAudioSource mainSource;
    ClientAudioSource clientSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
