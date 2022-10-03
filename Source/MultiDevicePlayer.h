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

    //==========================================================================
    void initialiseAudio (int numOutputChannels);
    void shutdownAudio();

    //==========================================================================
    void setSource (AudioSource* src) { mainSource.setSource (src); }

    //==========================================================================
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
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

    private:
        AudioSource* source = nullptr;
        MultiDevicePlayer& owner;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioSource)
    };

    class ClientAudioSource  : public AudioSource
    {
    public:
        explicit ClientAudioSource (MultiDevicePlayer& mdp) : owner (mdp) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

    private:
        MultiDevicePlayer& owner;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClientAudioSource)
    };

    friend class MainAudioSource;
    friend class ClientAudioSource;

    //==========================================================================
    MainAudioSource mainSource;
    ClientAudioSource clientSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
