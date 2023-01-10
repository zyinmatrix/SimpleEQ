/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
band1FreqSliderAttachment(audioProcessor.getAPVTS(), "Band1 Freq", band1FreqSlider),
band1GainSliderAttachment(audioProcessor.getAPVTS(), "Band1 Gain", band1GainSlider),
band1QualitySliderAttachment(audioProcessor.getAPVTS(), "Band1 Quality", band1QualitySlider),
band2FreqSliderAttachment(audioProcessor.getAPVTS(), "Band2 Freq", band2FreqSlider),
band2GainSliderAttachment(audioProcessor.getAPVTS(), "Band2 Gain", band2GainSlider),
band2QualitySliderAttachment(audioProcessor.getAPVTS(), "Band2 Quality", band2QualitySlider),
band3FreqSliderAttachment(audioProcessor.getAPVTS(), "Band3 Freq", band3FreqSlider),
band3GainSliderAttachment(audioProcessor.getAPVTS(), "Band3 Gain", band3GainSlider),
band3QualitySliderAttachment(audioProcessor.getAPVTS(), "Band3 Quality", band3QualitySlider),
lowCutFreqSliderAttachment(audioProcessor.getAPVTS(), "LowCut Freq", lowCutFreqSlider),
lowCutSlopeSliderAttachment(audioProcessor.getAPVTS(), "LowCut Slope", lowCutSlopeSlider),
highCutFreqSliderAttachment(audioProcessor.getAPVTS(), "HighCut Freq", highCutFreqSlider),
highCutSlopeSliderAttachment(audioProcessor.getAPVTS(), "HighCut Slope", highCutSlopeSlider)


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
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.36);
    auto w = responseArea.getWidth();
    
    // get chain element
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& band1 = monoChain.get<ChainPositions::Band1>();
    auto& band2 = monoChain.get<ChainPositions::Band2>();
    auto& band3 = monoChain.get<ChainPositions::Band3>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    // create a vector to store the magnitude response curve
    std::vector<double> mags;
    mags.resize(w);
    
    // calculate and store the magnitude for selected frequency
    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = juce::mapToLog10(double(i) / double(w), 20.0, 20000.0);
        
        // update curve
        if (!monoChain.isBypassed<ChainPositions::Band1>())
            mag *= band1.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!monoChain.isBypassed<ChainPositions::Band2>())
            mag *= band2.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!monoChain.isBypassed<ChainPositions::Band3>())
            mag *= band3.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        // store the magnitude in dB for current frequency
        mags[i] = juce::Decibels::gainToDecibels(mag);
    }
    
    juce::Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
                          
    // connect all the dots in magnitude
    for (int i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    

    g.setColour (juce::Colours::white);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
    
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
void SimpleEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void SimpleEQAudioProcessorEditor::timerCallback()
{
    if ( parametersChanged.compareAndSetBool(false, true) )
    {
        //update the monochain
        
        
        //signal a repaint
        repaint();
    }
}

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
