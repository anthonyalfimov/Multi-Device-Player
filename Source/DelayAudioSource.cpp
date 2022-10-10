/*
  ==============================================================================

    DelayAudioSource.cpp
    Created: 9 Oct 2022 9:54:05pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#include "DelayAudioSource.h"

DelayAudioSource::DelayAudioSource (int numChannels, double maxDelayInMs)
{
    setDelayBufferSize (numChannels, maxDelayInMs);
}

//==============================================================================
void DelayAudioSource::setDelayBufferSize (int numChannels, double maxDelayInMs)
{
    mNumChannels = numChannels;
    mMaxDelayInMs = maxDelayInMs;

    bufferResizePending = true;
}

//==============================================================================
void DelayAudioSource::setDelayInMs (float delayInMs)
{
    delaySmoothed.setTargetValue (mSampleRate * 0.001 * delayInMs);
}

void DelayAudioSource::setDelayInSamples (int delayInSamples)
{
    delaySmoothed.setTargetValue (delayInSamples);
}

//==============================================================================
void DelayAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mSampleRate = sampleRate;

    const dsp::ProcessSpec spec { mSampleRate,
                                  static_cast<uint32> (samplesPerBlockExpected),
                                  static_cast<uint32> (mNumChannels) };
    delayBuffer.prepare (spec);

    const int maxDelayInSamples
    = static_cast<int> (mSampleRate * 0.001 * mMaxDelayInMs) + 1;
    delayBuffer.setMaximumDelayInSamples (maxDelayInSamples);

    bufferResizePending = false;

    delaySmoothed.reset (sampleRate, delaySmoothingInSeconds);
}

void DelayAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    jassert (! bufferResizePending);

    const int numChannels = jmin (mNumChannels, bufferToFill.buffer->getNumChannels());

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
