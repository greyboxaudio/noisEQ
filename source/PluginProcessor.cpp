/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
noisEQAudioProcessor::noisEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
}

noisEQAudioProcessor::~noisEQAudioProcessor()
{
}

//==============================================================================
const juce::String noisEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool noisEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool noisEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool noisEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double noisEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int noisEQAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int noisEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void noisEQAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String noisEQAudioProcessor::getProgramName(int index)
{
    return {};
}

void noisEQAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void noisEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // set up filters & dsp elements
    lastSampleRate = static_cast<float>(sampleRate);
    float smoothFast{0.0005f};
    inputGainSmooth.reset(sampleRate, smoothFast);
    frequencySmoothL.reset(sampleRate, smoothFast);
    qfactorSmoothL.reset(sampleRate, smoothFast);
    gainSmoothL.reset(sampleRate, smoothFast);
    frequencySmoothLM.reset(sampleRate, smoothFast);
    qfactorSmoothLM.reset(sampleRate, smoothFast);
    gainSmoothLM.reset(sampleRate, smoothFast);
    frequencySmoothHM.reset(sampleRate, smoothFast);
    qfactorSmoothHM.reset(sampleRate, smoothFast);
    gainSmoothHM.reset(sampleRate, smoothFast);
    frequencySmoothH.reset(sampleRate, smoothFast);
    qfactorSmoothH.reset(sampleRate, smoothFast);
    gainSmoothH.reset(sampleRate, smoothFast);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    gainModule.prepare(spec);
    gainModule.reset();
    gainModule_d.prepare(spec);
    gainModule_d.reset();
    peakingEqualizerL.prepare(spec);
    peakingEqualizerL.reset();
    peakingEqualizerLM.prepare(spec);
    peakingEqualizerLM.reset();
    peakingEqualizerHM.prepare(spec);
    peakingEqualizerHM.reset();
    peakingEqualizerH.prepare(spec);
    peakingEqualizerH.reset();
    peakingEqualizerL_d.prepare(spec);
    peakingEqualizerL_d.reset();
    peakingEqualizerLM_d.prepare(spec);
    peakingEqualizerLM_d.reset();
    peakingEqualizerHM_d.prepare(spec);
    peakingEqualizerHM_d.reset();
    peakingEqualizerH_d.prepare(spec);
    peakingEqualizerH_d.reset();
}

void noisEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool noisEQAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void noisEQAudioProcessor::updateFilter()
{
    *peakingEqualizerL.state = juce::dsp::IIR::Coefficients<float>(coeffs_L[0], coeffs_L[1], coeffs_L[2], coeffs_L[3], coeffs_L[4], coeffs_L[5]);
    *peakingEqualizerLM.state = juce::dsp::IIR::Coefficients<float>(coeffs_LM[0], coeffs_LM[1], coeffs_LM[2], coeffs_LM[3], coeffs_LM[4], coeffs_LM[5]);
    *peakingEqualizerHM.state = juce::dsp::IIR::Coefficients<float>(coeffs_HM[0], coeffs_HM[1], coeffs_HM[2], coeffs_HM[3], coeffs_HM[4], coeffs_HM[5]);
    *peakingEqualizerH.state = juce::dsp::IIR::Coefficients<float>(coeffs_H[0], coeffs_H[1], coeffs_H[2], coeffs_H[3], coeffs_H[4], coeffs_H[5]);
    *peakingEqualizerL_d.state = juce::dsp::IIR::Coefficients<double>(coeffs_L_d[0], coeffs_L_d[1], coeffs_L_d[2], coeffs_L_d[3], coeffs_L_d[4], coeffs_L_d[5]);
    *peakingEqualizerLM_d.state = juce::dsp::IIR::Coefficients<double>(coeffs_LM_d[0], coeffs_LM_d[1], coeffs_LM_d[2], coeffs_LM_d[3], coeffs_LM_d[4], coeffs_LM_d[5]);
    *peakingEqualizerHM_d.state = juce::dsp::IIR::Coefficients<double>(coeffs_HM_d[0], coeffs_HM_d[1], coeffs_HM_d[2], coeffs_HM_d[3], coeffs_HM_d[4], coeffs_HM_d[5]);
    *peakingEqualizerH_d.state = juce::dsp::IIR::Coefficients<double>(coeffs_H_d[0], coeffs_H_d[1], coeffs_H_d[2], coeffs_H_d[3], coeffs_H_d[4], coeffs_H_d[5]);
}

void noisEQAudioProcessor::mvPeak(float freq, float gain, float qfactor, float sr)
{
    double pi = juce::MathConstants<double>::pi;
    double e = juce::MathConstants<double>::euler;
    double w0 = 2 * pi * freq / sr;
    double G = pow(10, gain / 20);
    double q = 1 / (2 * sqrt(G) * qfactor);
    double b0 = 1.0f;
    double b1{};
    if (q <= 1.0f)
    {
        b1 = -2 * pow(e, -1 * q * w0) * cos(sqrt(1 - pow(q, 2)) * w0);
    }
    else
    {
        b1 = -2 * pow(e, -1 * q * w0) * cosh(sqrt(pow(q, 2) - 1) * w0);
    }
    double b2 = pow(e, -2 * q * w0);
    double p0 = 1 - pow(sin(w0 / 2), 2);
    double p1 = pow(sin(w0 / 2), 2);
    double p2 = 4 * p0 * p1;
    double A0 = pow(1 + b1 + b2, 2);
    double A1 = pow(1 - b1 + b2, 2);
    double A2 = -4 * b2;
    double R1 = (A0 * p0 + A1 * p1 + A2 * p2) * pow(G, 2);
    double R2 = (-1 * A0 + A1 + 4 * (p0 - p1) * A2) * pow(G, 2);
    double B0 = A0;
    double B2 = (R1 - R2 * p1 - B0) / (4 * pow(p1, 2));
    double B1 = R2 + B0 + 4 * (p1 - p0) * B2;
    double W = 0.5 * (sqrt(B0) + sqrt(B1));
    double a0 = 0.5 * (W + sqrt(pow(W, 2) + B2));
    double a1 = 0.5 * (sqrt(B0) - sqrt(B1));
    double a2 = -1 * B2 / (4 * a0);
    coefficients[0] = a0;
    coefficients[1] = a1;
    coefficients[2] = a2;
    coefficients[3] = b0;
    coefficients[4] = b1;
    coefficients[5] = b2;
}
void noisEQAudioProcessor::mvShelf(float freq, float gain, float qfactor, float sr)
{
    double fc = freq / sr;
    double pi = juce::MathConstants<double>::pi;
    double g = (pow(10, (-1 * gain) / 20));
    if (abs(1 - gain) < 1e-6)
    {
        g = 1.00001;
    }
    // abbreviations
    double pihalf = pi * 0.5;
    double invg = 1.0 / g;
    // matching gain at Nyquist
    double fc4 = pow(fc, 4);
    double hny = (fc4 + g) / (fc4 + invg);
    // matching gain at f_1
    double f1 = fc / sqrt(0.160 + 1.543 * fc * fc);
    double f14 = pow(f1, 4);
    double h1 = (fc4 + f14 * g) / (fc4 + f14 * invg);
    double phi1 = pow(sin(pihalf * f1), 2);
    // matching gain at f_2
    double f2 = fc / sqrt(0.947 + 3.806 * fc * fc);
    double f24 = pow(f2, 4);
    double h2 = (fc4 + f24 * g) / (fc4 + f24 * invg);
    double phi2 = pow(sin(pihalf * f2), 2);
    // linear equations coefficients
    double d1 = (h1 - 1.0) * (1.0 - phi1);
    double c11 = -phi1 * d1;
    double c12 = phi1 * phi1 * (hny - h1);
    double d2 = (h2 - 1.0) * (1.0 - phi2);
    double c21 = -phi2 * d2;
    double c22 = phi2 * phi2 * (hny - h2);
    // linear equations solution
    double alfa1 = (c22 * d1 - c12 * d2) / (c11 * c22 - c12 * c21);
    double aa1 = (d1 - c11 * alfa1) / c12;
    double bb1 = hny * aa1;
    // compute A_2 and B_2
    double aa2 = 0.25 * (alfa1 - aa1);
    double bb2 = 0.25 * (alfa1 - bb1);
    // compute biquad coefficients scaled with 1/a_0
    double v = 0.5 * (1.0 + sqrt(aa1));
    double w = 0.5 * (1.0 + sqrt(bb1));
    double a0 = 0.5 * (v + sqrt(v * v + aa2));
    double inva0 = 1.0 / a0;
    double a1 = (1.0 - v) * inva0;
    double a2 = -0.25 * aa2 * inva0 * inva0;
    double b0 = (0.5 * (w + sqrt(w * w + bb2))) * inva0;
    double b1 = (1.0 - w) * inva0;
    double b2 = (-0.25 * bb2 / b0) * inva0 * inva0;
    // a0 = 1.0;
    coefficients[0] = a0;
    coefficients[1] = a1;
    coefficients[2] = a2;
    coefficients[3] = b0;
    coefficients[4] = b1;
    coefficients[5] = b2;
}
void noisEQAudioProcessor::biquadPeak(float freq, float gain, float qfactor, float sr)
{
    double pi = juce::MathConstants<double>::pi;
    double A = pow(10, (-1 * gain) / 40);
    double w0 = 2 * pi * (freq / sr);
    double sinw0 = sin(w0);
    double cosw0 = cos(w0);
    double Q = qfactor;
    double alpha = sinw0 / (2 * Q);
    double b0 = 1 + alpha * A;
    double b1 = -2 * cosw0;
    double b2 = 1 - alpha * A;
    double a0 = 1 + alpha / A;
    double a1 = -2 * cosw0;
    double a2 = 1 - alpha / A;
    coefficients[0] = a0;
    coefficients[1] = a1;
    coefficients[2] = a2;
    coefficients[3] = b0;
    coefficients[4] = b1;
    coefficients[5] = b2;
}
void noisEQAudioProcessor::biquadShelf(float freq, float gain, float qfactor, float sr)
{
    double pi = juce::MathConstants<double>::pi;
    double A = pow(10, (-1 * gain) / 40);
    double w0 = 2 * pi * ((2 * freq) / sr);
    double sinw0 = sin(w0);
    double cosw0 = cos(w0);
    double Q = 0.707;
    double alpha = sinw0 / (2 * Q);
    double b0 = A * ((A + 1) - (A - 1) * cosw0 + 2 * sqrt(A) * alpha);
    double b1 = 2 * A * ((A - 1) - (A + 1) * cosw0);
    double b2 = A * ((A + 1) - (A - 1) * cosw0 - 2 * sqrt(A) * alpha);
    double a0 = (A + 1) + (A - 1) * cosw0 + 2 * sqrt(A) * alpha;
    double a1 = -2 * ((A - 1) + (A + 1) * cosw0);
    double a2 = (A + 1) + (A - 1) * cosw0 - 2 * sqrt(A) * alpha;
}

void noisEQAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    auto bufferSize = buffer.getNumSamples();
    // prepare audio buffers
    inputBuffer.setSize(totalNumInputChannels, bufferSize);
    // set up dsp elements
    juce::dsp::AudioBlock<float> floatBlock(buffer);
    juce::dsp::AudioBlock<double> inputBlock(inputBuffer);
    // read smoothed parameters
    float frequencyValueL = *apvts.getRawParameterValue("FREQ_L");
    frequencySmoothL.setTargetValue(frequencyValueL);
    frequencyL = frequencySmoothL.getNextValue();
    float qfactorValueL = *apvts.getRawParameterValue("QFACTOR_L");
    qfactorSmoothL.setTargetValue(qfactorValueL);
    qfactorL = qfactorSmoothL.getNextValue();
    float gainValueL = *apvts.getRawParameterValue("GAIN_L");
    gainSmoothL.setTargetValue(gainValueL);
    gainL = gainSmoothL.getNextValue();
    float frequencyValueLM = *apvts.getRawParameterValue("FREQ_LM");
    frequencySmoothLM.setTargetValue(frequencyValueLM);
    frequencyLM = frequencySmoothLM.getNextValue();
    float qfactorValueLM = *apvts.getRawParameterValue("QFACTOR_LM");
    qfactorSmoothLM.setTargetValue(qfactorValueLM);
    qfactorLM = qfactorSmoothLM.getNextValue();
    float gainValueLM = *apvts.getRawParameterValue("GAIN_LM");
    gainSmoothLM.setTargetValue(gainValueLM);
    gainLM = gainSmoothLM.getNextValue();
    float frequencyValueHM = *apvts.getRawParameterValue("FREQ_HM");
    frequencySmoothHM.setTargetValue(frequencyValueHM);
    frequencyHM = frequencySmoothHM.getNextValue();
    float qfactorValueHM = *apvts.getRawParameterValue("QFACTOR_HM");
    qfactorSmoothHM.setTargetValue(qfactorValueHM);
    qfactorHM = qfactorSmoothHM.getNextValue();
    float gainValueHM = *apvts.getRawParameterValue("GAIN_HM");
    gainSmoothHM.setTargetValue(gainValueHM);
    gainHM = gainSmoothHM.getNextValue();
    float frequencyValueH = *apvts.getRawParameterValue("FREQ_H");
    frequencySmoothH.setTargetValue(frequencyValueH);
    frequencyH = frequencySmoothH.getNextValue();
    float qfactorValueH = *apvts.getRawParameterValue("QFACTOR_H");
    qfactorSmoothH.setTargetValue(qfactorValueH);
    qfactorH = qfactorSmoothH.getNextValue();
    float gainValueH = *apvts.getRawParameterValue("GAIN_H");
    gainSmoothH.setTargetValue(gainValueH);
    gainH = gainSmoothH.getNextValue();

    // Low Frequency
    bool peakButtonStateL = *apvts.getRawParameterValue("PEAK_L");
    biquadPeak(frequencyL, gainL, qfactorL, lastSampleRate);
    coeffs_L[0] = static_cast<float>(coefficients[0]);
    coeffs_L[1] = static_cast<float>(coefficients[1]);
    coeffs_L[2] = static_cast<float>(coefficients[2]);
    coeffs_L[3] = static_cast<float>(coefficients[3]);
    coeffs_L[4] = static_cast<float>(coefficients[4]);
    coeffs_L[5] = static_cast<float>(coefficients[5]);
    coeffs_L_d[0] = coefficients[0];
    coeffs_L_d[1] = coefficients[1];
    coeffs_L_d[2] = coefficients[2];
    coeffs_L_d[3] = coefficients[3];
    coeffs_L_d[4] = coefficients[4];
    coeffs_L_d[5] = coefficients[5];

    // Low Mid Frequency
    bool peakButtonStateLM = *apvts.getRawParameterValue("PEAK_LM");
    biquadPeak(frequencyLM, gainLM, qfactorLM, lastSampleRate);
    coeffs_LM[0] = static_cast<float>(coefficients[0]);
    coeffs_LM[1] = static_cast<float>(coefficients[1]);
    coeffs_LM[2] = static_cast<float>(coefficients[2]);
    coeffs_LM[3] = static_cast<float>(coefficients[3]);
    coeffs_LM[4] = static_cast<float>(coefficients[4]);
    coeffs_LM[5] = static_cast<float>(coefficients[5]);
    coeffs_LM_d[0] = coefficients[0];
    coeffs_LM_d[1] = coefficients[1];
    coeffs_LM_d[2] = coefficients[2];
    coeffs_LM_d[3] = coefficients[3];
    coeffs_LM_d[4] = coefficients[4];
    coeffs_LM_d[5] = coefficients[5];

    // High Mid Frequency
    bool peakButtonStateHM = *apvts.getRawParameterValue("PEAK_HM");
    mvPeak(frequencyHM, gainHM, qfactorHM, lastSampleRate);
    coeffs_HM[0] = static_cast<float>(coefficients[0]);
    coeffs_HM[1] = static_cast<float>(coefficients[1]);
    coeffs_HM[2] = static_cast<float>(coefficients[2]);
    coeffs_HM[3] = static_cast<float>(coefficients[3]);
    coeffs_HM[4] = static_cast<float>(coefficients[4]);
    coeffs_HM[5] = static_cast<float>(coefficients[5]);
    coeffs_HM_d[0] = coefficients[0];
    coeffs_HM_d[1] = coefficients[1];
    coeffs_HM_d[2] = coefficients[2];
    coeffs_HM_d[3] = coefficients[3];
    coeffs_HM_d[4] = coefficients[4];
    coeffs_HM_d[5] = coefficients[5];

    // High Frequency
    bool peakButtonStateH = *apvts.getRawParameterValue("PEAK_H");
    mvPeak(frequencyH, gainH, qfactorH, lastSampleRate);
    coeffs_H[0] = static_cast<float>(coefficients[0]);
    coeffs_H[1] = static_cast<float>(coefficients[1]);
    coeffs_H[2] = static_cast<float>(coefficients[2]);
    coeffs_H[3] = static_cast<float>(coefficients[3]);
    coeffs_H[4] = static_cast<float>(coefficients[4]);
    coeffs_H[5] = static_cast<float>(coefficients[5]);
    coeffs_H_d[0] = coefficients[0];
    coeffs_H_d[1] = coefficients[1];
    coeffs_H_d[2] = coefficients[2];
    coeffs_H_d[3] = coefficients[3];
    coeffs_H_d[4] = coefficients[4];
    coeffs_H_d[5] = coefficients[5];

    // update filters
    updateFilter();
    // clear buffers
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
        inputBuffer.clear(channel, 0, bufferSize);
    // copy audio buffer to 64bit inputBuffer
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        for (int i = 0; i < bufferSize; ++i)
        {
            inputBuffer.setSample(channel, i, static_cast<double>(buffer.getSample(channel, i)));
        }
    }
    // apply filter
    if (peakButtonStateL)
    {
        peakingEqualizerL.process(juce::dsp::ProcessContextReplacing<float>(floatBlock));
        peakingEqualizerL_d.process(juce::dsp::ProcessContextReplacing<double>(inputBlock));
    }
    if (peakButtonStateLM)
    {
        peakingEqualizerLM.process(juce::dsp::ProcessContextReplacing<float>(floatBlock));
        peakingEqualizerLM_d.process(juce::dsp::ProcessContextReplacing<double>(inputBlock));
    }
    if (peakButtonStateHM)
    {
        peakingEqualizerHM.process(juce::dsp::ProcessContextReplacing<float>(floatBlock));
        peakingEqualizerHM_d.process(juce::dsp::ProcessContextReplacing<double>(inputBlock));
    }
    if (peakButtonStateH)
    {
        peakingEqualizerH.process(juce::dsp::ProcessContextReplacing<float>(floatBlock));
        peakingEqualizerH_d.process(juce::dsp::ProcessContextReplacing<double>(inputBlock));
    }

    // apply input gain
    float inputGainValue = *apvts.getRawParameterValue("INPUT");
    inputGainSmooth.setTargetValue(inputGainValue);
    inputGain = inputGainSmooth.getNextValue();
    inputGain = static_cast<float>(pow(10, inputGain / 20));
    gainModule.setGainLinear(inputGain);
    gainModule_d.setGainLinear(inputGain);
    gainModule.process(juce::dsp::ProcessContextReplacing<float>(floatBlock));
    gainModule_d.process(juce::dsp::ProcessContextReplacing<double>(inputBlock));

    bool precisionButtonState = *apvts.getRawParameterValue("PRECISION");
    // write input buffer back to main buffer
    if (precisionButtonState==true)
    {
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        {
            // buffer.copyFrom(channel, 0, inputBuffer, channel, 0, bufferSize);
            for (int i = 0; i < bufferSize; ++i)
            {
                buffer.setSample(channel, i, static_cast<float>(inputBuffer.getSample(channel, i)));
            }
        }
    }
}

//==============================================================================
bool noisEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *noisEQAudioProcessor::createEditor()
{
    return new noisEQAudioProcessorEditor(*this);
}

//==============================================================================
void noisEQAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void noisEQAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new noisEQAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout noisEQAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT", "inputGain", -18.0f, 18.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("PEAK_L", "Peak_L", true));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("PEAK_LM", "Peak_LM", true));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("PEAK_HM", "Peak_HM", true));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("PEAK_H", "Peak_H", true));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>("PRECISION", "Precision", false));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_L", "Gain L", -18.5f, 18.5f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ_L", "Frequency L", 33.0f, 450.0f, 150.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ_LM", "Frequency LM", 170.0f, 1500.0f, 1500.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ_HM", "Frequency HM", 1700.0f, 16000.0f, 15000.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("FREQ_H", "Frequency H", 2000.0f, 21000.0f, 4500.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("QFACTOR_L", "QFactor L", 0.2f, 1.9f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_LM", "Gain LM", -18.5f, 18.5f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("QFACTOR_LM", "QFactor LM", 0.2f, 1.9f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_HM", "Gain HM", -18.5f, 18.5f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("QFACTOR_HM", "QFactor HM", 0.2f, 1.9f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN_H", "Gain H", -18.5f, 18.5f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>("QFACTOR_H", "QFactor H", 0.2f, 1.9f, 1.0f));
    return {parameters.begin(), parameters.end()};
}