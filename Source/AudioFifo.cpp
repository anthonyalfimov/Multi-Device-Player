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
int AudioFifo::push (const AudioSourceChannelInfo& inInfo)
{
    return pushWithRamp (inInfo, 1.0f, 1.0f);
}

int AudioFifo::pop (const AudioSourceChannelInfo& outInfo)
{
    return popWithRamp (outInfo, 1.0f, 1.0f);
}

int AudioFifo::pushWithRamp (const AudioSourceChannelInfo& inInfo,
                             float startGain, float endGain)
{
    const auto status = fifoManager.write (inInfo.numSamples);

    const auto* inBuffer = inInfo.buffer;
    const auto channelsRequired = buffer.getNumChannels();
    const auto channelsToProcess = jmin (inBuffer->getNumChannels(), channelsRequired);
    
    const float midGain = getMidGain (startGain, endGain,
                                      status.blockSize1, status.blockSize2);

    if (status.blockSize1 > 0)
    {
        const int inStartIndex1 = inInfo.startSample;

        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            buffer.copyFromWithRamp (ch, status.startIndex1,
                                     inBuffer->getReadPointer (ch, inStartIndex1),
                                     status.blockSize1,
                                     startGain, midGain);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            buffer.clear (ch, status.startIndex1, status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        const int inStartIndex2 = inInfo.startSample + status.blockSize1;

        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            buffer.copyFromWithRamp (ch, status.startIndex2,
                                     inBuffer->getReadPointer (ch, inStartIndex2),
                                     status.blockSize2,
                                     midGain, endGain);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            buffer.clear (ch, status.startIndex2, status.blockSize2);
        }
    }

    return status.blockSize1 + status.blockSize2;
}

int AudioFifo::popWithRamp (const AudioSourceChannelInfo& outInfo,
                            float startGain, float endGain)
{
    const auto status = fifoManager.read (outInfo.numSamples);

    auto* outBuffer = outInfo.buffer;
    const auto channelsRequired = outBuffer->getNumChannels();
    const auto channelsToProcess = jmin (buffer.getNumChannels(), channelsRequired);

    const float midGain = getMidGain (startGain, endGain,
                                      status.blockSize1, status.blockSize2);

    if (status.blockSize1 > 0)
    {
        const int outStartIndex1 = outInfo.startSample;

        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            outBuffer->copyFromWithRamp (ch, outStartIndex1,
                                         buffer.getReadPointer (ch, status.startIndex1),
                                         status.blockSize1,
                                         startGain, midGain);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            outBuffer->clear (ch, outStartIndex1, status.blockSize1);
        }
    }

    if (status.blockSize2 > 0)
    {
        const int outStartIndex2 = outInfo.startSample + status.blockSize1;

        for (int ch = 0; ch < channelsToProcess; ++ch)
        {
            outBuffer->copyFromWithRamp (ch, outStartIndex2,
                                         buffer.getReadPointer (ch, status.startIndex2),
                                         status.blockSize2,
                                         midGain, endGain);
        }

        // Clear any remaining channels:
        for (int ch = channelsToProcess; ch < channelsRequired; ++ch)
        {
            outBuffer->clear (ch, outStartIndex2, status.blockSize2);
        }
    }

    return status.blockSize1 + status.blockSize2;
}

float AudioFifo::getMidGain (float startGain, float endGain,
                             int blockSize1, int blockSize2)
{
    if (startGain == endGain)
        return startGain;

    float midRatio = static_cast<float> (blockSize1) / (blockSize1 + blockSize2);

    return jmap (midRatio, startGain, endGain);
}
