/*
  ==============================================================================

    AudioFifo.h
    Created: 28 Sep 2022 7:40:25pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
    Thread-safe resizable FIFO for audio samples
*/
class AudioFifo
{
public:
    AudioFifo() = default;

    //==========================================================================
    /** [Realtime] [Thread-safe]
        Returns the number of channels of audio data that this FIFO contains.
    */
    int getNumChannels() const { return buffer.getNumChannels(); }

    /** [Realtime] [Thread-safe]
        Returns the number of samples allocated in each of the FIFO's channels.
    */
    int getTotalSize() const;

    /** [Realtime] [Thread-safe]
        Returns the number of samples that can currently be added to the FIFO
        without it overflowing.
    */
    int getFreeSpace() const { return fifoManager.getFreeSpace(); }

    /** [Realtime] [Thread-safe]
        Returns the number of samples that can currently be read from the FIFO.
    */
    int getNumReady() const { return fifoManager.getNumReady(); }

    //==========================================================================
    /** [Non-realtime] [Non-thread-safe]
        Changes the FIFO's size or number of channels.

        This can expand or contract the FIFO's length, and add or remove channels.

        If the required memory can't be allocated, this will throw a std::bad_alloc
        exception.

        @param newNumChannels       the new number of channels.
        @param newNumSamples        the new number of samples.
     */
    void setSize (int newNumChannels, int newNumSamples);

    /** [Realtime] [Non-thread-safe]
        Clears the FIFO.
    */
    void reset();

    //==========================================================================
    /** [Realtime] [Thread-safe]
        Push samples to the FIFO.

        @returns    the number of samples pushed to the FIFO.
    */
    int push (const AudioSourceChannelInfo& inInfo);

    /** [Realtime] [Thread-safe]
        Pop samples from the FIFO.

        @returns    the number of samples popped from the FIFO.
    */
    int pop (const AudioSourceChannelInfo& outInfo);

    /** [Realtime] [Thread-safe]
     Push samples to the FIFO and apply a gain ramp.

     @returns    the number of samples pushed to the FIFO.
     */
    int pushWithRamp (const AudioSourceChannelInfo& inInfo,
                      float startGain, float endGain);

    /** [Realtime] [Thread-safe]
     Pop samples from the FIFO and apply a gain ramp.

     @returns    the number of samples popped from the FIFO.
     */
    int popWithRamp (const AudioSourceChannelInfo& outInfo,
                     float startGain, float endGain);

private:
    AbstractFifo fifoManager { defaultSize };
    AudioBuffer<float> buffer { 2, defaultSize };

    inline static constexpr int defaultSize = 512;

    float getMidGain (float startGain, float endGain, int blockSize1, int blockSize2);

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFifo)
};
