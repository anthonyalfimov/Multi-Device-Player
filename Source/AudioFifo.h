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
    Lock-free FIFO for audio samples
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

        @return     Returns true if all requested samples have been pushed to
                    the FIFO.
    */
    bool push (const AudioSourceChannelInfo& inBuffer);

    /** [Realtime] [Thread-safe]
        Pop samples from the FIFO.

        @return     Returns true if all requested samples have been popped from
                    the FIFO.
    */
    bool pop (AudioSourceChannelInfo& outBuffer);

private:
    AbstractFifo fifoManager { 32 };
    AudioBuffer<float> buffer { 2, 32 };

    //==========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFifo)
};
