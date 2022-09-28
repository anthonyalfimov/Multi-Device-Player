/*
  ==============================================================================

    AudioFifo.cpp
    Created: 28 Sep 2022 7:40:25pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "AudioFifo.h"

int AudioFifo::getTotalSize() const
{
    jassert (buffer.getNumSamples() == fifoManager.getTotalSize());
    return fifoManager.getTotalSize();
}

//==========================================================================
void AudioFifo::setSize (int newNumChannels, int newNumSamples)
{
    // TODO: Use a spin lock to resize the FIFO?
    //       Pushing and popping could fail and return "false" if lock can't be
    //       acquired. FIFO data (total size, free space and available items)
    //       can all return failure states while FIFO is being resized.

    fifoManager.setTotalSize (newNumSamples);
    buffer.setSize (newNumChannels, newNumSamples, false, true, false);
}

void AudioFifo::reset()
{
    // TODO: Use a spin lock to reset the FIFO?

    fifoManager.reset();
    buffer.clear();
}

//==========================================================================
bool AudioFifo::push (const AudioSourceChannelInfo& inBuffer)
{
    const auto status = fifoManager.write (inBuffer.numSamples);

    // TODO: handle mismatched number of channels
    const auto numChannels = buffer.getNumChannels();
    jassert (numChannels == inBuffer.buffer->getNumChannels());

    if (status.blockSize1 > 0)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            buffer.copyFrom (i, status.startIndex1,
                             *inBuffer.buffer,
                             i, inBuffer.startSample,
                             status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            buffer.copyFrom (i, status.startIndex2,
                             *inBuffer.buffer,
                             i, inBuffer.startSample + status.blockSize1,
                             status.blockSize2);
        }
    }

    return (status.blockSize1 + status.blockSize2) == inBuffer.numSamples;
}

bool AudioFifo::pop (AudioSourceChannelInfo& outBuffer)
{
    const auto status = fifoManager.read (outBuffer.numSamples);

    // TODO: handle mismatched number of channels
    const auto numChannels = buffer.getNumChannels();
    jassert (numChannels == outBuffer.buffer->getNumChannels());

    if (status.blockSize1 > 0)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            outBuffer.buffer->copyFrom (i, outBuffer.startSample,
                                        buffer,
                                        i, status.startIndex1,
                                        status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        for (int i = 0; i < numChannels; ++i)
        {
            outBuffer.buffer->copyFrom (i, outBuffer.startSample + status.blockSize1,
                                        buffer,
                                        i, status.startIndex2,
                                        status.blockSize2);
        }
    }

    return (status.blockSize1 + status.blockSize2) == outBuffer.numSamples;
}
