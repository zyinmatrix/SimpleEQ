/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
// MODIFIED BY zyinmatrix
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    
    auto chainSettings = getChainSettings(apvts);
    
    // calculate peak filters' coefficients
    auto band1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                 chainSettings.band1Freq,
                                                                                 chainSettings.band1Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band1GainInDecibles));
    
    auto band2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                 chainSettings.band2Freq,
                                                                                 chainSettings.band2Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band2GainInDecibles));

    auto band3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                 chainSettings.band3Freq,
                                                                                 chainSettings.band3Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band3GainInDecibles));
    
    // set peak filter's coefficients
    *leftChain.get<ChainPositions::Band1>().coefficients = *band1Coefficients;
    *rightChain.get<ChainPositions::Band1>().coefficients = *band1Coefficients;
    
    *leftChain.get<ChainPositions::Band2>().coefficients = *band2Coefficients;
    *rightChain.get<ChainPositions::Band2>().coefficients = *band2Coefficients;
    
    *leftChain.get<ChainPositions::Band3>().coefficients = *band3Coefficients;
    *rightChain.get<ChainPositions::Band3>().coefficients = *band3Coefficients;
    
    // calculate cut filters' coefficients
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                          sampleRate,
                                                                                                          2 * (chainSettings.lowCutSlope+1));
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                          sampleRate,
                                                                                                          2 * (chainSettings.highCutSlope+1));
    
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);
    
    switch( chainSettings.lowCutSlope )
    {
        case Slope_48:
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
        case Slope_36:
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
        case Slope_24:
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
        case Slope_12:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
        
    }
    
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
// MODIFIED by zyinmatrix

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(apvts);
    
    // calculate peak filters' coefficients
    auto band1Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                 chainSettings.band1Freq,
                                                                                 chainSettings.band1Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band1GainInDecibles));
    
    auto band2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                 chainSettings.band2Freq,
                                                                                 chainSettings.band2Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band2GainInDecibles));

    auto band3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                 chainSettings.band3Freq,
                                                                                 chainSettings.band3Quality,
                                                                                 juce::Decibels::decibelsToGain(chainSettings.band3GainInDecibles));
    // set peak filter's coefficients
    *leftChain.get<ChainPositions::Band1>().coefficients = *band1Coefficients;
    *rightChain.get<ChainPositions::Band1>().coefficients = *band1Coefficients;
    
    *leftChain.get<ChainPositions::Band2>().coefficients = *band2Coefficients;
    *rightChain.get<ChainPositions::Band2>().coefficients = *band2Coefficients;
    
    *leftChain.get<ChainPositions::Band3>().coefficients = *band3Coefficients;
    *rightChain.get<ChainPositions::Band3>().coefficients = *band3Coefficients;
    
    // calculate cut filters' coefficients
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                          getSampleRate(),
                                                                                                          2 * (chainSettings.lowCutSlope+1));
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                          getSampleRate(),
                                                                                                          2 * (chainSettings.highCutSlope+1));
    
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);
    
    switch( chainSettings.lowCutSlope )
    {
        case Slope_48:
            *leftLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            *rightLowCut.get<3>().coefficients = *lowCutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
        case Slope_36:
            *leftLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *rightLowCut.get<2>().coefficients = *lowCutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
        case Slope_24:
            *leftLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *rightLowCut.get<1>().coefficients = *lowCutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
        case Slope_12:
            *leftLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *rightLowCut.get<0>().coefficients = *lowCutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
        
    }
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);
    
    switch( chainSettings.highCutSlope )
    {
        case Slope_48:
            *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
            leftHighCut.setBypassed<3>(false);
            *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
            rightHighCut.setBypassed<3>(false);
        case Slope_36:
            *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
            leftHighCut.setBypassed<2>(false);
            *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
            rightHighCut.setBypassed<2>(false);
        case Slope_24:
            *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
            leftHighCut.setBypassed<1>(false);
            *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
            rightHighCut.setBypassed<1>(false);
        case Slope_12:
            *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
            leftHighCut.setBypassed<0>(false);
            *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
            rightHighCut.setBypassed<0>(false);
        
    }
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    // context wrapper around the audio blocks
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    // pass the context to filter chain for processing
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    
//    std::cout << chainSettings.band1Freq;
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
//    return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// MODIFIED by zyinmatrix
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    // low/high cut frequency and slope
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    
    settings.lowCutSlope = static_cast<Slope> (apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope> (apvts.getRawParameterValue("HighCut Slope")->load());
    
    // band 1 frequency, gain, and quality
    settings.band1Freq = apvts.getRawParameterValue("Band1 Freq")->load();
    settings.band1GainInDecibles= apvts.getRawParameterValue("Band1 Gain")->load();
    settings.band1Quality = apvts.getRawParameterValue("Band1 Quality")->load();
    
    
    // band 2 frequency, gain, and quality
    settings.band2Freq = apvts.getRawParameterValue("Band2 Freq")->load();
    settings.band2GainInDecibles= apvts.getRawParameterValue("Band2 Gain")->load();
    settings.band2Quality = apvts.getRawParameterValue("Band2 Quality")->load();
    
    // band 3 frequency, gain, and quality
    settings.band3Freq = apvts.getRawParameterValue("Band3 Freq")->load();
    settings.band3GainInDecibles= apvts.getRawParameterValue("Band3 Gain")->load();
    settings.band3Quality = apvts.getRawParameterValue("Band3 Quality")->load();
 
    return settings;
}


juce::AudioProcessorValueTreeState::ParameterLayout
SimpleEQAudioProcessor::createParameterLayout()
{
    // use AudioParameterFloat type for the parameters
//    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // low cut, high cut frequency
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LowCut Freq", 1), "LowCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("HighCut Freq", 1), "HighCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20000.f));
    
    // band filter 1 frequency, gain, and quality
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band1 Freq", 1), "Band1 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 250.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band1 Gain", 1), "Band1 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band1 Quality", 1), "Band1 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
    
    // band filter 2 frequency, gain, and quality
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band2 Freq", 1), "Band2 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 2500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band2 Gain", 1), "Band2 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band2 Quality", 1), "Band2 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
    
    // band filter 3 frequency, gain, and quality
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band3 Freq", 1), "Band3 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 8000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band3 Gain", 1), "Band3 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Band3 Quality", 1), "Band3 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
    
    // string array for slope
    juce::StringArray slopeArray;
    for (int i = 0; i<4; ++i){
        juce::String str;
        str << (12 + i*12);
        str << " db/Oct";
        slopeArray.add(str);
    }
    
    // low cut, high cut slopes
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LowCut Slope", 1), "LowCut Slope", slopeArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("HighCut Slope", 1), "HighCut Slope", slopeArray, 0));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
