/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//MODIFIED by zyinmatrix

// enum for slope settings
enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// struct that stores the value of all parameters
struct ChainSettings
{
    float band1Freq{0}, band1GainInDecibles{0}, band1Quality{1.f};
    float band2Freq{0}, band2GainInDecibles{0}, band2Quality{1.f};
    float band3Freq{0}, band3GainInDecibles{0}, band3Quality{1.f};
    float lowCutFreq{0}, highCutFreq{0};
    Slope lowCutSlope{Slope::Slope_12}, highCutSlope{Slope::Slope_12};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);


//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
// MODIFIED by zyinmatrix
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

private:
// MODIFIED by zyinmatrix
    using Filter = juce::dsp::IIR::Filter<float>;
    // use processor chain to conect filters
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, CutFilter>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions
    {
      LowCut,
      Band1,
      Band2,
      Band3,
      HighCut
    };
    
    void updateFilters();
    using Coefficients = Filter::CoefficientsPtr;
   
    // does not use any member variables, so set it to static
    static void updateCoefficients(Coefficients &old, const Coefficients &replacements);
    

    template<int Index, typename ChainType, typename CoefficientType>
    void update(ChainType& chain, const CoefficientType& coefficients)
    {
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
        chain.template setBypassed<Index>(false);
    }

    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& cutChain,
                        const CoefficientType& cutCoefficients,
                        const int& slope)
    {
        // bypass all indivitual filters in left/righ high cut filter chains
        cutChain.template setBypassed<0>(true);
        cutChain.template setBypassed<1>(true);
        cutChain.template setBypassed<2>(true);
        cutChain.template setBypassed<3>(true);
        
        // enable and set coefficients for indivitual filters according to the high cut slope
        switch( slope )
        {
            case Slope_48:
            {
                update<3>(cutChain, cutCoefficients);
            }
            case Slope_36:
            {
                update<2>(cutChain, cutCoefficients);
            }
            case Slope_24:
            {
                update<1>(cutChain, cutCoefficients);
            }
            case Slope_12:
            {
                update<0>(cutChain, cutCoefficients);
            }
        }
    }
    void updateBandFilters(const ChainSettings &chainSettings);
    void updateLowCutFilter(const ChainSettings &chainSettings);
    void updateHighCutFilter(const ChainSettings &chainSettings);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
