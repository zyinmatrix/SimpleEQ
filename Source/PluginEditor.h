/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// MODIFIED by zyinmatrix

/* LookAndFeel struct for RotarySliderWithLabels */
struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                                   int x, int y, int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                           juce::Slider& slider) override;
};


/* struct for custom sliders */
struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<float> getSliderBounds() const;
    int getTextHeight() const { return 12; }
    juce::String getDisplayString() const;
    
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
    
    
    
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
    void updateCurve();
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
    RotarySliderWithLabels band1FreqSlider, band1GainSlider, band1QualitySlider,
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
