/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for( auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    setSize (600, 300);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    // reserve area for frequency analyser
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.36);
    
    // reserve area for cut filters
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 1/5);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 1/4);
    
    // reserve area for band filters
    auto band1Area = bounds.removeFromLeft(bounds.getWidth() * 1/3);
    auto band2Area = bounds.removeFromLeft(bounds.getWidth() * 1/2);
    auto band3Area = bounds;
    
    // set bounds for sliders
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 1/2));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 1/2));
    highCutSlopeSlider.setBounds(highCutArea);
    
    band1FreqSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 1/3));
    band1GainSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 1/2));
    band1QualitySlider.setBounds(band1Area);
    
    band2FreqSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 1/3));
    band2GainSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 1/2));
    band2QualitySlider.setBounds(band2Area);
    
    band3FreqSlider.setBounds(band3Area.removeFromTop(band3Area.getHeight() * 1/3));
    band3GainSlider.setBounds(band3Area.removeFromTop(band3Area.getHeight() * 1/2));
    band3QualitySlider.setBounds(band3Area);
    
}

// MODIFIED by zyinmatrix
std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &band1FreqSlider, &band1GainSlider, &band1QualitySlider,
        &band2FreqSlider, &band2GainSlider, &band2QualitySlider,
        &band3FreqSlider, &band3GainSlider, &band3QualitySlider,
        &lowCutFreqSlider, &lowCutSlopeSlider,
        &highCutFreqSlider, &highCutSlopeSlider
    };
}
