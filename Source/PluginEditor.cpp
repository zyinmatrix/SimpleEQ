/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawToggleButton (juce::Graphics& g,
                       juce::ToggleButton& button,
                       bool shouldDrawButtonAsHighlighted,
                       bool shouldDrawButtonAsDown)
{
    
    if (auto* pb = dynamic_cast<PowerButton*>(&button))
    {
        juce::Path powerButton;
        
        auto bounds = button.getLocalBounds();
        auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.54;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size/2,
                                  size/2,
                                  0.f,
                                  juce::degreesToRadians(ang),
                                  juce::degreesToRadians(360.f-ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        juce::PathStrokeType pst (2.f, juce::PathStrokeType::JointStyle::curved);
        
        auto color = button.getToggleState() ? juce::Colour(90u, 90u, 90u) : juce::Colour(138u, 190u, 110u);
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
    //    g.drawEllipse(r, 1.f);
    }
    else if (auto* pb = dynamic_cast<AnalyzerButton*>(&button))
    {
        auto color = button.getToggleState() ? juce::Colour(138u, 190u, 110u) : juce::Colour(90u, 90u, 90u);
        g.setColour(color);
        
        auto bounds = button.getLocalBounds();
        g.drawRect(bounds);
        
        auto innerRect = bounds.reduced(1);
        
        juce::Path randomPath;
        juce::Random rand;
        
        randomPath.startNewSubPath(innerRect.getX(),
                                   innerRect.getY() + innerRect.getHeight()/2);
        
        for (auto x = innerRect.getX() + 1; x < innerRect.getRight(); x+=2)
        {
            randomPath.lineTo(x, innerRect.getY() + innerRect.getHeight() * rand.nextFloat());
        }
        
        g.strokePath(randomPath, juce::PathStrokeType(1.f));
    }
//    g.setColour(juce::Colour(138u, 190u, 110u));
//    g.drawRect(button.getLocalBounds());
    
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g,
                               int x, int y, int width, int height,
                               float sliderPosProportional,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                       juce::Slider& slider)
{
    
    // check if is RotarySlidersWithLables
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height);
        
        g.setColour(juce::Colour(255u, 134u, 182u));
        g.fillEllipse(bounds);
        
        g.setColour(juce::Colour(255u, 255u, 255u));
        g.drawEllipse(bounds, 1.f);
        auto center = bounds.getCentre();
        
        juce::Path p;
        
        juce::Rectangle<float> r;
        r.setLeft(center.getX()-1);
        r.setRight(center.getX()+1);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()-rswl->getTextHeight() * 1.2);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngRad = juce::jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(juce::AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
       
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        g.setColour(juce::Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}


void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    auto startAng = juce::degreesToRadians(-135.f);
    auto endAng = juce::degreesToRadians(135.f);
    
    auto range = getRange();
    auto rangeWithSkew = juce::NormalisableRange<float>(range.getStart(), range.getEnd(), 1.f, 0.25f);
    auto sliderBounds = getSliderBounds();
    
    // draw boxes around bounds
//    g.setColour(juce::Colour(255u, 255u, 255u));
//    g.drawRect(getLocalBounds());
//    g.drawRect(sliderBounds);
    
    if(suffix=="Hz")
    {
        getLookAndFeel().drawRotarySlider(g,
                                          sliderBounds.getX(), sliderBounds.getY(),
                                          sliderBounds.getWidth(), sliderBounds.getHeight(),
                                          rangeWithSkew.convertTo0to1(getValue()),
                                          startAng, endAng, *this);
    }
    else {
        getLookAndFeel().drawRotarySlider(g,
                                          sliderBounds.getX(), sliderBounds.getY(),
                                          sliderBounds.getWidth(), sliderBounds.getHeight(),
                                          juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                          startAng, endAng, *this);
    }
    
    // draw labels for each slider
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

//    g.setColour(juce::Colour(255u, 255u, 255u));
    g.setColour(juce::Colour(138u, 190u, 110u));
    
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = juce::jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight(), ang);
        
        juce::Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight() * 0.6);
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<float> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    
    juce::Rectangle<float> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), size/2 + 2);
    
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
//    return juce::String(getValue());
    
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        // if over 1000
            // devide by 1000
            // addK = true
        float val = getValue();
        if (val >= 1000.f)
        {
            val /= 1000.f;
            addK = true;
        }
        
        // do not display value after decimal point
        val = int(val);
        
        str = juce::String(val, 0);
    }
    else
    {
        jassertfalse; // this should not happen!
    }
    
    // add suffix
    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";
        str << suffix;
    }
    
    return str;
}

//==============================================================================
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    
    while (channelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (channelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();
                        
                        // shift old samples in monoBuffer
                        juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                                          monoBuffer.getReadPointer(0, size),
                                                          monoBuffer.getNumSamples() - size);
                        
                        // copy new samples to monoBuffer
                        juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                                          tempIncomingBuffer.getReadPointer(0, 0),
                                                          size);
                        
                        channelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    
    /*
     if there are FFT buffers to pull
        if we can pull a buffer
            generate a path
     
     */

    auto fftSize = channelFFTDataGenerator.getFFTSize();
    
    /*
     48000 / 2048 = 23hz <- this is the bin width
     */
    
    const auto binWidh = sampleRate / (double) fftSize;
    
    while (channelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (channelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidh, -48.f);
        }
    }
    
    /*
     while there are paths that can be pull
        pull as many as we can
            display the most recent path
     */
    
    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(ChannelFFTPath);
    }
    
    
}


// member functions for ResponseCurveComponent
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p)
: audioProcessor (p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)

{
    // add listeners to all parameters
    const auto& params = audioProcessor.getParameters();
    for( auto param : params)
    {
        param->addListener(this);
    }
    
    
    
    updateCurve();
    startTimerHz(30);
}
ResponseCurveComponent::~ResponseCurveComponent()
{
    // remove listeners to all parameters
    const auto& params = audioProcessor.getParameters();
    for( auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    
    auto fftBounds = getAnalysisArea().toFloat();
    auto sampleRate = audioProcessor.getSampleRate();
    
    leftPathProducer.process(fftBounds, sampleRate);
    rightPathProducer.process(fftBounds, sampleRate);
    
    if ( parametersChanged.compareAndSetBool(false, true) )
    {
//        DBG( "params changed " );
        updateCurve(); // update the monochain
//        repaint(); //signal a repaint
    }
    
    repaint();
}

void ResponseCurveComponent::updateCurve()
{
    auto chainSettings = getChainSettings(audioProcessor.getAPVTS());
    auto sampleRate = audioProcessor.getSampleRate();
    
    // update bypass states
    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    monoChain.setBypassed<ChainPositions::Band1>(chainSettings.band1Bypassed);
    monoChain.setBypassed<ChainPositions::Band2>(chainSettings.band2Bypassed);
    monoChain.setBypassed<ChainPositions::Band3>(chainSettings.band3Bypassed);
    
    //update coefficients
    auto band1Coefficients = makeBand1Filter(chainSettings, sampleRate);
    auto band2Coefficients = makeBand2Filter(chainSettings, sampleRate);
    auto band3Coefficients = makeBand3Filter(chainSettings, sampleRate);
    
    updateCoefficients(monoChain.get<Band1>().coefficients, band1Coefficients);
    updateCoefficients(monoChain.get<Band2>().coefficients, band2Coefficients);
    updateCoefficients(monoChain.get<Band3>().coefficients, band3Coefficients);
    
    // calculate cut filters' coefficients
    auto lowCutCoefficients = makeLowCutFilter( chainSettings, sampleRate);
    auto highCutCoefficients = makeHighCutFilter( chainSettings, sampleRate);
    // update low cut filter chains
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
//    auto responseArea = getRenderArea();
    auto responseArea = getAnalysisArea();
    auto w = responseArea.getWidth();
    
    // draw background grid
    g.drawImage(background, getLocalBounds().toFloat());
    
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
        
        if (!monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (!lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        // store the magnitude in dB for current frequency
        mags[i] = juce::Decibels::gainToDecibels(mag);
    }
    
    juce::Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        auto magnitude = input;
//        if (magnitude > 24) magnitude = 24;
//        else if (magnitude < -24) magnitude = -24;
        return juce::jmap(magnitude, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
                          
    // connect all the dots in magnitude
    for (int i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    
    auto leftChannelFFTPath = leftPathProducer.getPath();
    auto rightChannelFFTPath = rightPathProducer.getPath();
    
    leftChannelFFTPath.applyTransform(juce::AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    rightChannelFFTPath.applyTransform(juce::AffineTransform().translation(responseArea.getX(), responseArea.getY()));
    
    g.setColour(juce::Colour(90u, 207u, 243u));
    g.strokePath(leftChannelFFTPath, juce::PathStrokeType(1.f));
    
    g.setColour(juce::Colour(41u, 135u, 248u));
//    g.setColour (juce::Colours::white);
    g.strokePath(rightChannelFFTPath, juce::PathStrokeType(1.f));
    
    
    g.setColour (juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

void ResponseCurveComponent::resized()
{
    background = juce::Image(juce::Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    
    juce::Graphics g(background);
    
//    g.setColour(juce::Colour(176u, 250u, 235u));
    g.setColour(juce::Colour(163u, 230u, 216u));

    // Array for frequency and gain
    juce::Array<float> freqs
    {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
    juce::Array<float> gains
    {
        -24, -12, 0, 12, 24
    };
    
    // Get areas
    auto renderArea = getRenderArea();
    auto analysisArea = getAnalysisArea();
    auto top = analysisArea.getY();
    auto bottom = analysisArea.getBottom();
    auto left = analysisArea.getX();
    auto right = analysisArea.getRight();
    auto width = analysisArea.getWidth();
    
    juce::Array<float> normXs;
    juce::Array<float> normYs;
    
    for (auto f: freqs)
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        g.drawVerticalLine(left + width * normX, top, bottom); // draw vertial lines
        
        normXs.add(left + width * normX); // store X coordinates for freq labels
    }

    for (auto gain : gains)
    {
        auto y = juce::jmap(gain, -24.f, 24.f, float(bottom), float(top));
        g.drawHorizontalLine(y, float(left), float(right)); // draw horizontal lines
        
        normYs.add(y);
    }
    
    // draw freq labels for grid
    const int fontHeight = 10;
    g.setFont(fontHeight);
    g.setColour(juce::Colour(138u, 190u, 110u));
    
    for (int i = 0; i < freqs.size(); ++i)
    {
        juce::String str;
        auto f = freqs[i];
        auto x = normXs[i];
        bool addK = false;
        
        if (f >= 1000.f)
        {
            f /= 1000.f;
            addK = true;
        }

        str << f;
        if (addK) str << "k";
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        juce::Rectangle<int> r;
        
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(2);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    // draw gain labels for grid
    for (int i = 0; i < gains.size(); ++i)
    {
        juce::String str;
        auto gain = gains[i];
        auto y = normYs[i];
        
        if (gain > 0) str << "+";
        str << gain;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        juce::Rectangle<int> r;
        
        r.setSize(textWidth, fontHeight);
        r.setCentre(0, y);
        r.setX(getWidth() - textWidth - 3);
    
        g.drawFittedText(str, r, juce::Justification::centred, 1);
        
        str.clear();
        str << gain-24;
        textWidth = g.getCurrentFont().getStringWidth(str);
        
        r.setSize(textWidth, fontHeight);
        r.setCentre(0, y);
        r.setX(3);
    
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
//    g.drawRect(getLocalBounds());
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    
//    bounds.reduce(12, 9);
    bounds.removeFromTop(15);
    bounds.removeFromBottom(3);
    bounds.removeFromLeft(24);
    bounds.removeFromRight(24);
    
    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(2);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(2);
    bounds.removeFromRight(2);
    return bounds;
}

//==============================================================================

SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

responseCurveComponent(audioProcessor),
band1FreqSlider(*audioProcessor.getAPVTS().getParameter("Band1 Freq"), "Hz"),
band1GainSlider(*audioProcessor.getAPVTS().getParameter("Band1 Gain"), "dB"),
band1QualitySlider(*audioProcessor.getAPVTS().getParameter("Band1 Quality"), ""),
band2FreqSlider(*audioProcessor.getAPVTS().getParameter("Band2 Freq"), "Hz"),
band2GainSlider(*audioProcessor.getAPVTS().getParameter("Band2 Gain"), "dB"),
band2QualitySlider(*audioProcessor.getAPVTS().getParameter("Band2 Quality"), ""),
band3FreqSlider(*audioProcessor.getAPVTS().getParameter("Band3 Freq"), "Hz"),
band3GainSlider(*audioProcessor.getAPVTS().getParameter("Band3 Gain"), "dB"),
band3QualitySlider(*audioProcessor.getAPVTS().getParameter("Band3 Quality"), ""),
lowCutFreqSlider(*audioProcessor.getAPVTS().getParameter("LowCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.getAPVTS().getParameter("LowCut Slope"), "dB/Oct"),
highCutFreqSlider(*audioProcessor.getAPVTS().getParameter("HighCut Freq"), "Hz"),
highCutSlopeSlider(*audioProcessor.getAPVTS().getParameter("HighCut Slope"), "dB/Oct"),
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
highCutSlopeSliderAttachment(audioProcessor.getAPVTS(), "HighCut Slope", highCutSlopeSlider),
lowCutBypassButtonAttachment(audioProcessor.getAPVTS(), "LowCut Bypassed", lowCutBypassButton),
highCutBypassButtonAttachment(audioProcessor.getAPVTS(), "HighCut Bypassed", highCutBypassButton),
band1BypassButtonAttachment(audioProcessor.getAPVTS(), "Band1 Bypassed", band1BypassButton),
band2BypassButtonAttachment(audioProcessor.getAPVTS(), "Band2 Bypassed", band2BypassButton),
band3BypassButtonAttachment(audioProcessor.getAPVTS(), "Band3 Bypassed", band3BypassButton),
analyzerEnabledAttachment(audioProcessor.getAPVTS(), "Analyzer Enabled", analyzerEnabledButton)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    juce::String startFreq{"20Hz"};
    juce::String endFreq{"20kHz"};
    juce::String startGain{"-24dB"};
    juce::String endGain{"+24dB"};
    juce::String startQuality{"0.1"};
    juce::String endQuality{"10"};
    juce::String startSlope{"12"};
    juce::String endSlope{"48"};
    
    // add lables for sliders
    band1FreqSlider.labels.add({0.f, startFreq});
    band1FreqSlider.labels.add({1.f, endFreq});
    band1GainSlider.labels.add({0.f, startGain});
    band1GainSlider.labels.add({1.f, endGain});
    band1QualitySlider.labels.add({0.f, startQuality});
    band1QualitySlider.labels.add({1.f, endQuality});
    
    band2FreqSlider.labels.add({0.f, startFreq});
    band2FreqSlider.labels.add({1.f, endFreq});
    band2GainSlider.labels.add({0.f, startGain});
    band2GainSlider.labels.add({1.f, endGain});
    band2QualitySlider.labels.add({0.f, startQuality});
    band2QualitySlider.labels.add({1.f, endQuality});
    
    band3FreqSlider.labels.add({0.f, startFreq});
    band3FreqSlider.labels.add({1.f, endFreq});
    band3GainSlider.labels.add({0.f, startGain});
    band3GainSlider.labels.add({1.f, endGain});
    band3QualitySlider.labels.add({0.f, startQuality});
    band3QualitySlider.labels.add({1.f, endQuality});
    
    lowCutFreqSlider.labels.add({0.f, startFreq});
    lowCutFreqSlider.labels.add({1.f, endFreq});
    lowCutSlopeSlider.labels.add({0.f, startSlope});
    lowCutSlopeSlider.labels.add({1.f, endSlope});
    
    highCutFreqSlider.labels.add({0.f, startFreq});
    highCutFreqSlider.labels.add({1.f, endFreq});
    highCutSlopeSlider.labels.add({0.f, startSlope});
    highCutSlopeSlider.labels.add({1.f, endSlope});

    
    for( auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    band1BypassButton.setLookAndFeel(&lnfToggle);
    band2BypassButton.setLookAndFeel(&lnfToggle);
    band3BypassButton.setLookAndFeel(&lnfToggle);
    lowCutBypassButton.setLookAndFeel(&lnfToggle);
    highCutBypassButton.setLookAndFeel(&lnfToggle);
    analyzerEnabledButton.setLookAndFeel(&lnfToggle);
    
    int seed = 45;
    setSize (15*seed, 9*seed);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    band1BypassButton.setLookAndFeel(nullptr);
    band2BypassButton.setLookAndFeel(nullptr);
    band3BypassButton.setLookAndFeel(nullptr);
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    analyzerEnabledButton.setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colour(31u, 31u, 36u));
//    g.fillAll (juce::Colour(47u, 50u, 57u));
//    g.fillAll (juce::Colour(27u, 27u, 36u));
    
    
    
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    // reserve area for analyzer button
    auto topArea = bounds.removeFromTop(bounds.getHeight() * 0.09);
    auto analyzerEnabledArea = topArea.removeFromLeft(topArea.getWidth() * 0.18);
    analyzerEnabledArea = analyzerEnabledArea.withSizeKeepingCentre(analyzerEnabledArea.getWidth() * 0.9, analyzerEnabledArea.getHeight() * 0.6);
    
    // reserve area for frequency analyser
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.36);
    responseCurveComponent.setBounds(responseArea);
    
    
//    auto spaceMid = bounds.removeFromTop(bounds.getHeight() * 0.018);
    auto spaceBottom = bounds.removeFromBottom(bounds.getHeight() * 0.018);
    
    // reserve area for cut filters
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 1/5);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 1/4);
    
    // reserve area for band filters
    auto band1Area = bounds.removeFromLeft(bounds.getWidth() * 1/3);
    auto band2Area = bounds.removeFromLeft(bounds.getWidth() * 1/2);
    auto band3Area = bounds;
    
    // set bounds for sliders and buttons
    analyzerEnabledButton.setBounds(analyzerEnabledArea);
    
    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 1/9));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 1/2));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutBypassButton.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 1/9));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 1/2));
    highCutSlopeSlider.setBounds(highCutArea);
    
    band1BypassButton.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 1/9));
    band1FreqSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 1/3));
    band1GainSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 1/2));
    band1QualitySlider.setBounds(band1Area);
    
    band2BypassButton.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 1/9));
    band2FreqSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 1/3));
    band2GainSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 1/2));
    band2QualitySlider.setBounds(band2Area);
    
    band3BypassButton.setBounds(band3Area.removeFromTop(band3Area.getHeight() * 1/9));
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
        &highCutFreqSlider, &highCutSlopeSlider,
        &responseCurveComponent,
        &lowCutBypassButton, &highCutBypassButton,
        &band1BypassButton, &band2BypassButton, &band3BypassButton,
        &analyzerEnabledButton
    };
}
