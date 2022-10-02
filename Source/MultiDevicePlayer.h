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
    SpinLock popMutex;

    //==========================================================================
    class MainAudioSource  : public AudioSource
    {
    public:
        explicit MainAudioSource (MultiDevicePlayer& mdp) : owner (mdp) {}

        //======================================================================
        void setSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {
            if (source != nullptr)
                source->prepareToPlay (samplesPerBlockExpected, sampleRate);

            // TODO:  Determine the number of channels based on device configuration
            // TODO:  Consider using a function for buffer size?
            SpinLock::ScopedLockType lock (owner.popMutex);
            owner.fifoBuffer.setSize (2, samplesPerBlockExpected * 4);
        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            if (source != nullptr)
                source->getNextAudioBlock (bufferToFill);
            else
                bufferToFill.clearActiveBufferRegion();

            owner.fifoBuffer.push (bufferToFill);
        }

        void releaseResources() override
        {
            if (source != nullptr)
                source->releaseResources();

            //sharedBuffer.reset();
        }
    private:
        AudioSource* source = nullptr;
        MultiDevicePlayer& owner;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioSource)
    };

    friend class MainAudioSource;

    class ClientAudioSource  : public AudioSource
    {
    public:
        explicit ClientAudioSource (MultiDevicePlayer& mdp) : owner (mdp) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {

        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            SpinLock::ScopedTryLockType lock (owner.popMutex);

            if (lock.isLocked())
                owner.fifoBuffer.pop (bufferToFill);
            else
                bufferToFill.clearActiveBufferRegion();
        }

        void releaseResources() override
        {

        }

    private:
        MultiDevicePlayer& owner;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClientAudioSource)
    };

    friend class ClientAudioSource;

    //==========================================================================
    MainAudioSource mainSource;
    ClientAudioSource clientSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
