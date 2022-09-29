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
    // Push and pop will require locking for the duration of the whole operation.
    // Are there other way to resize the FIFO in a thread-safe manner when
    // one of the devices is changed?

    buffer.setSize (newNumChannels, newNumSamples, false, true, false);
    fifoManager.setTotalSize (newNumSamples);
}

void AudioFifo::reset()
{
    // TODO: Use a spin lock to reset the FIFO?

    buffer.clear();
    fifoManager.reset();
}

//==========================================================================
int AudioFifo::push (const AudioSourceChannelInfo& inBuffer)
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

    return status.blockSize1 + status.blockSize2;
}

int AudioFifo::pop (const AudioSourceChannelInfo& outBuffer)
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

    return status.blockSize1 + status.blockSize2;
}
