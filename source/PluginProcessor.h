/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class noisEQAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
    , public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    noisEQAudioProcessor();
    ~noisEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void updateFilter();
    void biquadPeak(float freq, float gain, float qfactor, float sr);
    void biquadShelf(float freq, float gain, float qfactor, float sr);
    void mvPeak(float freq, float gain, float qfactor, float sr);
    void mvShelf(float freq, float gain, float qfactor, float sr);
    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    juce::AudioBuffer<double> inputBuffer;

    juce::LinearSmoothedValue<float> inputGainSmooth{};
    juce::LinearSmoothedValue<float> frequencySmoothL{};
    juce::LinearSmoothedValue<float> qfactorSmoothL{};
    juce::LinearSmoothedValue<float> gainSmoothL{};
    juce::LinearSmoothedValue<float> frequencySmoothLM{};
    juce::LinearSmoothedValue<float> qfactorSmoothLM{};
    juce::LinearSmoothedValue<float> gainSmoothLM{};
    juce::LinearSmoothedValue<float> frequencySmoothHM{};
    juce::LinearSmoothedValue<float> qfactorSmoothHM{};
    juce::LinearSmoothedValue<float> gainSmoothHM{};
    juce::LinearSmoothedValue<float> frequencySmoothH{};
    juce::LinearSmoothedValue<float> qfactorSmoothH{};
    juce::LinearSmoothedValue<float> gainSmoothH{};

    juce::dsp::Gain<float> gainModule;
    juce::dsp::Gain<double> gainModule_d;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float>, juce::dsp::IIR::Coefficients <float>> peakingEqualizerL;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float>, juce::dsp::IIR::Coefficients <float>> peakingEqualizerLM;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float>, juce::dsp::IIR::Coefficients <float>> peakingEqualizerHM;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <float>, juce::dsp::IIR::Coefficients <float>> peakingEqualizerH;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <double>, juce::dsp::IIR::Coefficients <double>> peakingEqualizerL_d;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <double>, juce::dsp::IIR::Coefficients <double>> peakingEqualizerLM_d;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <double>, juce::dsp::IIR::Coefficients <double>> peakingEqualizerHM_d;
    juce::dsp::ProcessorDuplicator <juce::dsp::IIR::Filter <double>, juce::dsp::IIR::Coefficients <double>> peakingEqualizerH_d;

    float inputGain{};
    float frequencyL{};
    float qfactorL{};
    float gainL{};
    float frequencyLM{};
    float qfactorLM{};
    float gainLM{};
    float frequencyHM{};
    float qfactorHM{};
    float gainHM{};
    float frequencyH{};
    float qfactorH{};
    float gainH{};
    float lastSampleRate{};
    double coefficients[6]{};
    float coeffs_L[6]{};
    float coeffs_LM[6]{};
    float coeffs_HM[6]{};
    float coeffs_H[6]{};
    double coeffs_L_d[6]{};
    double coeffs_LM_d[6]{};
    double coeffs_H_d[6]{};
    double coeffs_HM_d[6]{};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(noisEQAudioProcessor)
};