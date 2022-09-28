/*
  ==============================================================================

    MultiDevicePlayer.h
    Created: 19 Sep 2022 7:51:03pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class MultiDevicePlayer
{
public:
    MultiDevicePlayer();
    ~MultiDevicePlayer();

    void setAudioSource (AudioSource* src) { mainSource.setAudioSource (src); }

    AudioDeviceManager mainDeviceManager;
    AudioDeviceManager linkedDeviceManager;

private:
    AudioSourcePlayer mainSourcePlayer;
    AudioSourcePlayer linkedSourcePlayer;

    // TODO: Consider using AbstractFifo with an AudioBuffer
    using DelayBuffer = dsp::DelayLine<double, dsp::DelayLineInterpolationTypes::Thiran>;
    DelayBuffer sharedBuffer;

    //==========================================================================
    class MainAudioSource  : public AudioSource
    {
    public:
        explicit MainAudioSource (DelayBuffer& sb) : sharedBuffer (sb) {}

        //======================================================================
        void setAudioSource (AudioSource* src) { source = src; }

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {
            if (source != nullptr)
                source->prepareToPlay (samplesPerBlockExpected, sampleRate);

            const dsp::ProcessSpec spec
            {
                sampleRate,
                static_cast<uint32> (samplesPerBlockExpected),
                static_cast<uint32> (2)
            };

            sharedBuffer.prepare (spec);

            sharedBuffer.setMaximumDelayInSamples (10 * samplesPerBlockExpected);
        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            if (source != nullptr)
                source->getNextAudioBlock (bufferToFill);
            else
                bufferToFill.clearActiveBufferRegion();

            const auto** inBuffer = bufferToFill.buffer->getArrayOfReadPointers();

            for (int i = 0; i < bufferToFill.numSamples; ++i)
            {
                const int sample = bufferToFill.startSample + i;

                sharedBuffer.pushSample (0, inBuffer[0][sample]);
                sharedBuffer.pushSample (1, inBuffer[1][sample]);
            }
        }

        void releaseResources() override
        {
            if (source != nullptr)
                source->releaseResources();

            sharedBuffer.reset();
        }
    private:
        AudioSource* source = nullptr;
        DelayBuffer& sharedBuffer;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAudioSource)
    };

    class LinkedAudioSource  : public AudioSource
    {
    public:
        explicit LinkedAudioSource (DelayBuffer& sb) : sharedBuffer (sb) {}

        //======================================================================
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
        {
            delayInSamples = samplesPerBlockExpected;
        }

        void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
        {
            auto** outBuffer = bufferToFill.buffer->getArrayOfWritePointers();

            for (int i = 0; i < bufferToFill.numSamples; ++i)
            {
                const int sample = bufferToFill.startSample + i;

                outBuffer[0][sample] = sharedBuffer.popSample (0, delayInSamples);
                outBuffer[1][sample] = sharedBuffer.popSample (1, delayInSamples);
            }
        }

        void releaseResources() override
        {

        }

    private:
        DelayBuffer& sharedBuffer;

        double delayInSamples = 0;

        //======================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinkedAudioSource)
    };

    //==========================================================================
    MainAudioSource mainSource;
    LinkedAudioSource linkedSource;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiDevicePlayer)
};
