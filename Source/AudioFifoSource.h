/*
  ==============================================================================

    AudioFifoSource.h
    Created: 18 Oct 2022 6:30:36pm
    Author:  Anthony Alfimov

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioFifo.h"

class AudioFifoSource  : public AudioSource
{
public:
    explicit AudioFifoSource (AudioFifo& af) : fifo (af) {}

    //==========================================================================
    void setGainRamp (float startGain, float endGain)
    {
        startPopGain = startGain;
        endPopGain = endGain;
    }

    //==========================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        const int numPopped = fifo.popWithRamp (bufferToFill, startPopGain, endPopGain);

        if (numPopped < bufferToFill.numSamples)
        {
            const int startSampleToClear = bufferToFill.startSample + numPopped;
            const int numSamplesToClear = bufferToFill.numSamples - numPopped;
            bufferToFill.buffer->clear (startSampleToClear, numSamplesToClear);
        }
    }
    
    void releaseResources() override {}

private:
    AudioFifo& fifo;

    float startPopGain = 1.0f;
    float endPopGain = 1.0f;
};
