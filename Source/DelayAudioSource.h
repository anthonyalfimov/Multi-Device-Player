/*
  ==============================================================================

    DelayAudioSource.h
    Created: 9 Oct 2022 9:54:05pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DelayAudioSource  : public AudioSource
{
public:
    /** Default constructor of a DelayAudioSource object.

        setDelayBufferSize() method must be called before the first
        prepareToPlay() call. Otherwise default values for the buffer size
        will be used.
    */
    DelayAudioSource() = default;

    /** Creates a DelayAudioSource and sets its delay buffer size.
    */
    DelayAudioSource (int numChannels, int maxDelayInSamples);

    //==========================================================================
    /** [Realtime] [Non-thread-safe]
        Sets the size of the internal delay buffer. The changes to the buffer
        size are only applied when prepareToPlay() is called.

        If the object was created using the default constructor,
        this method must be called before the first prepareToPlay() call.
        Otherwise default values for the buffer size will be used.
    */
    void setDelayBufferSize (int numChannels, int maxDelayInSamples);

    /** [Realtime] [Non-thread-safe]
        Returns true when internal delay buffer has been resized to the
        requested size.
     */
    bool isDelayBufferReady() const { return ! bufferResizePending; }

    //==========================================================================
    void setDelay (int delayInSamples);

    //==========================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    int channels = 2;
    int maxDelay = 512;

    bool bufferResizePending = true;

    //==========================================================================
    SmoothedValue<float> delaySmoothed;
    using InterpolationMethod = dsp::DelayLineInterpolationTypes::Lagrange3rd;
    dsp::DelayLine <float, InterpolationMethod> delayBuffer;

    //==========================================================================
    inline static constexpr float delaySmoothingInSeconds = 0.05f;

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioSource);
};
