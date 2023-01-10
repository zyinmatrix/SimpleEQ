/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// MODIFIED by zyinmatrix
// Create a structure for our custom sliders
struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
    
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      public juce::AudioProcessorParameter::Listener,
                                      public juce::Timer
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {} ;
    
    void timerCallback() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;
    
// MODIFIED by zyinmatrix
    juce::Atomic<bool> parametersChanged {false};
    
    // Create sliders
    CustomRotarySlider band1FreqSlider, band1GainSlider, band1QualitySlider,
    band2FreqSlider, band2GainSlider, band2QualitySlider,
    band3FreqSlider, band3GainSlider, band3QualitySlider,
    lowCutFreqSlider, lowCutSlopeSlider,
    highCutFreqSlider, highCutSlopeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment band1FreqSliderAttachment, band1GainSliderAttachment, band1QualitySliderAttachment,
    band2FreqSliderAttachment, band2GainSliderAttachment, band2QualitySliderAttachment,
    band3FreqSliderAttachment, band3GainSliderAttachment, band3QualitySliderAttachment,
    lowCutFreqSliderAttachment, lowCutSlopeSliderAttachment,
    highCutFreqSliderAttachment, highCutSlopeSliderAttachment;
    
    std::vector<juce::Component*> getComps();
    
    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
