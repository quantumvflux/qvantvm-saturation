class SaturationDSP
{
    public:
        void prepare (double sampleRate, int samplesPerBlock);
        void setDrive (float gainLinear);
        float processSample (float input);

    private:
        float currentDrive = 1.0f;
        double currentSampleRate = 44100.0;
        float prevInput = 0.0f;
        float prevOutput = 0.0f;
};
