/*
  ==============================================================================

    DelayAudioSource.cpp
    Created: 9 Oct 2022 9:54:05pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DelayAudioSource.h"

DelayAudioSource::DelayAudioSource (int numChannels, int maxDelayInSamples)
{
    setDelayBufferSize (numChannels, maxDelayInSamples);
}

//==============================================================================
void DelayAudioSource::setDelayBufferSize (int numChannels, int maxDelayInSamples)
{
    channels = numChannels;
    maxDelay = maxDelayInSamples;

    bufferResizePending = true;
}

//==============================================================================
void DelayAudioSource::setDelay (int delayInSamples)
{
    delaySmoothed.setTargetValue (jmin (delayInSamples, maxDelay));
}

//==============================================================================
void DelayAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const dsp::ProcessSpec spec { sampleRate,
                                  static_cast<uint32> (samplesPerBlockExpected),
                                  static_cast<uint32> (channels) };
    delayBuffer.prepare (spec);
    delayBuffer.setMaximumDelayInSamples (maxDelay + 1);

    bufferResizePending = false;

    delaySmoothed.reset (sampleRate, delaySmoothingInSeconds);
}

void DelayAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    jassert (! bufferResizePending);

    const int numChannels = jmin (channels, bufferToFill.buffer->getNumChannels());

    const auto** inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
    auto** outBuffer = bufferToFill.buffer->getArrayOfWritePointers();

    const auto startSample = bufferToFill.startSample;
    const auto endSample = startSample + bufferToFill.numSamples;

    for (int i = startSample; i < endSample; ++i)
    {
        const auto delayValue = delaySmoothed.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            delayBuffer.pushSample (ch, inBuffer[ch][i]);
            outBuffer[ch][i] = delayBuffer.popSample (ch, delayValue);
        }
    }
}

void DelayAudioSource::releaseResources()
{
    delayBuffer.reset();
    delaySmoothed.setCurrentAndTargetValue (0.0f);
}
