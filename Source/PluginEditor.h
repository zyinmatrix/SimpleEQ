/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// MODIFIED by zyinmatrix
/* structure for custom sliders */
struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
    
};

/* structure for response curve */
struct ResponseCurveComponent : juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
    ResponseCurveComponent(SimpleEQAudioProcessor&);
    ~ResponseCurveComponent();
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {} ;
    
    void timerCallback() override;
    
    void paint (juce::Graphics&) override;
    
private:
    SimpleEQAudioProcessor& audioProcessor;
    MonoChain monoChain;
    juce::Atomic<bool> parametersChanged {false};
};

//==============================================================================
/**
*/
class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor
                                      
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEQAudioProcessor& audioProcessor;
    
// MODIFIED by zyinmatrix
    ResponseCurveComponent responseCurveComponent;
    
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
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};
