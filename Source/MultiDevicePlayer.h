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
    void initialiseAudio (AudioSource* src, int numOutputChannels);
    void shutdownAudio();

    //==========================================================================
    AudioDeviceManager mainDeviceManager;
    AudioDeviceManager linkedDeviceManager;

private:
    AudioSourcePlayer mainSourcePlayer;
    AudioSourcePlayer linkedSourcePlayer;

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

    class LinkedAudioSource  : public AudioSource
    {
    public:
        explicit LinkedAudioSource (MultiDevicePlayer& mdp) : owner (mdp) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

    private:
        MultiDevicePlayer& owner;

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
