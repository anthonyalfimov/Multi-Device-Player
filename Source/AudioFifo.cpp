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
    buffer.setSize (newNumChannels, newNumSamples, false, true, false);
    fifoManager.setTotalSize (newNumSamples);
}

void AudioFifo::reset()
{
    buffer.clear();
    fifoManager.reset();
}

//==========================================================================
int AudioFifo::push (const AudioSourceChannelInfo& inBuffer)
{
    const auto status = fifoManager.write (inBuffer.numSamples);

    const auto channelsRequired = buffer.getNumChannels();
    const auto channelsToProcess = jmin (inBuffer.buffer->getNumChannels(),
                                         channelsRequired);

    if (status.blockSize1 > 0)
    {
        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            buffer.copyFrom (ch, status.startIndex1,
                             *inBuffer.buffer,
                             ch, inBuffer.startSample,
                             status.blockSize1);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            buffer.clear (ch, status.startIndex1, status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            buffer.copyFrom (ch, status.startIndex2,
                             *inBuffer.buffer,
                             ch, inBuffer.startSample + status.blockSize1,
                             status.blockSize2);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            buffer.clear (ch, status.startIndex2, status.blockSize2);
        }
    }

    return status.blockSize1 + status.blockSize2;
}

int AudioFifo::pop (const AudioSourceChannelInfo& outBuffer)
{
    const auto status = fifoManager.read (outBuffer.numSamples);

    const auto channelsRequired = outBuffer.buffer->getNumChannels();
    const auto channelsToProcess = jmin (buffer.getNumChannels(),
                                         channelsRequired);

    if (status.blockSize1 > 0)
    {
        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            outBuffer.buffer->copyFrom (ch, outBuffer.startSample,
                                        buffer,
                                        ch, status.startIndex1,
                                        status.blockSize1);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            outBuffer.buffer->clear (ch, outBuffer.startSample, status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            outBuffer.buffer->copyFrom (ch, outBuffer.startSample + status.blockSize1,
                                        buffer,
                                        ch, status.startIndex2,
                                        status.blockSize2);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            outBuffer.buffer->clear (ch, outBuffer.startSample + status.blockSize1,
                                     status.blockSize2);
        }
    }

    return status.blockSize1 + status.blockSize2;
}
