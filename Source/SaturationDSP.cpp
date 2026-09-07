#include "SaturationDSP.h"
#include <cmath>

void SaturationDSP::prepare (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
};

void SaturationDSP::setDrive (float gainLinear)
{
    currentDrive = gainLinear;
};

float SaturationDSP::processSample (float input)
{
    float x = input * currentDrive;
    float absX = std::abs (x);

    float offset = 0.1f * currentDrive * absX;
    float shaped = (std::tanh (x) + std::tanh (x + offset)) * 0.5f;

    float output = shaped - prevOutput + 0.995f * prevOutput;
    prevInput = shaped;
    prevOutput = output;

    return output;
};
