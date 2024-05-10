/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MysynthpracAudioProcessorEditor::MysynthpracAudioProcessorEditor(MysynthpracAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), waveTypeMenuAttatchment(p.state, "wavetype", waveTypeMenu), ladderModeMenuAttatchment(p.state, "laddermode", ladderModeMenu),
    ladderButtonAttatchment(p.state, "ladderbutton", ladderButton),
    attackAttatchment(p.state, "attack", attackSlider),
    decayAttatchment(p.state, "decay", decaySlider),
    sustainAttatchment(p.state, "sustain", sustainSlider),
    releaseAttatchment(p.state, "release", releaseSlider),
    masterVolumeAttatchment(p.state, "volume", masterVolumeSlider),
    ladderCutOffAttatchment(p.state, "laddercutoff", ladderCutOffSlider),
    ladderDriveAttatchment(p.state, "ladderdrive", ladderDriveSlider),
    ladderResAttatchment(p.state, "ladderresonance", ladderResSlider),
    scope(1)

{
    //Wave type combobox
    waveTypeMenu.addItem("Sine", 1);
    waveTypeMenu.addItem("Triangle", 2);
    waveTypeMenu.addItem("Saw", 3);
    waveTypeMenu.addItem("Square", 4);
    addAndMakeVisible(&waveTypeMenu);

    //Ladder filter on/off button
    ladderButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    ladderButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    ladderButton.setClickingTogglesState(true);
    ladderButton.onClick = [&]()
    {
        const auto buttonText = ladderButton.getToggleState() ? "Filter On!" : "Filter Off!";
        ladderButton.setButtonText(buttonText);

    };
    addAndMakeVisible(&ladderButton);
    
    //Ladder filter combobox
    ladderModeMenu.addItem("LPF12", 1);
    ladderModeMenu.addItem("HPF12", 2);
    ladderModeMenu.addItem("BPF12", 3);
    ladderModeMenu.addItem("LPF24", 4);
    ladderModeMenu.addItem("HPF24", 5);
    ladderModeMenu.addItem("BPF24", 6);
    addAndMakeVisible(&ladderModeMenu);

    waveTypeMenu.setJustificationType(juce::Justification::centred);
    
    //ADSR Control Sliders
    addAndMakeVisible(attackSlider);
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    attackSlider.setPopupDisplayEnabled(true, true, this);
    
    addAndMakeVisible(decaySlider);
    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    decaySlider.setPopupDisplayEnabled(true, true, this);
    
    addAndMakeVisible(sustainSlider);
    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    sustainSlider.setPopupDisplayEnabled(true, true, this);
    
    addAndMakeVisible(releaseSlider);
    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    releaseSlider.setPopupDisplayEnabled(true, true, this);

    //Master Volume
    addAndMakeVisible(masterVolumeSlider);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    masterVolumeSlider.setPopupDisplayEnabled(true, true, this);

    addAndMakeVisible(volumeLabel);
    volumeLabel.attachToComponent(&masterVolumeSlider, false);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    
    //Ladder filter control Sliders
    addAndMakeVisible(ladderCutOffSlider);
    ladderCutOffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ladderCutOffSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    ladderCutOffSlider.setPopupDisplayEnabled(true, true, this);

    addAndMakeVisible(ladderResSlider);
    ladderResSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ladderResSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    ladderResSlider.setPopupDisplayEnabled(true, true, this);

    addAndMakeVisible(ladderDriveSlider);
    ladderDriveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ladderDriveSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    ladderDriveSlider.setPopupDisplayEnabled(true, true, this);

    //Ladder fileter control labels
    addAndMakeVisible(ladderCuttOffLabel);
    ladderCuttOffLabel.attachToComponent(&ladderCutOffSlider, false);
    ladderCuttOffLabel.setText("Hz", juce::dontSendNotification);
    ladderCuttOffLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(ladderResonanceLabel);
    ladderResonanceLabel.attachToComponent(&ladderResSlider, false);
    ladderResonanceLabel.setText("Res", juce::dontSendNotification);
    ladderResonanceLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(ladderDriveLabel);
    ladderDriveLabel.attachToComponent(&ladderDriveSlider, false);
    ladderDriveLabel.setText("Drive", juce::dontSendNotification);
    ladderDriveLabel.setJustificationType(juce::Justification::centred);

    //ADSR Control Labels
    addAndMakeVisible(attackLabel);
    attackLabel.attachToComponent(&attackSlider, false);
    attackLabel.setText("A", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(decayLabel);
    decayLabel.attachToComponent(&decaySlider, false);
    decayLabel.setText("D", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(sustainLabel);
    sustainLabel.attachToComponent(&sustainSlider, false);
    sustainLabel.setText("S", juce::dontSendNotification);
    sustainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(releaseLabel);
    releaseLabel.attachToComponent(&releaseSlider, false);
    releaseLabel.setText("R", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);


    waveTypeMenu.setSelectedId(1);
    ladderModeMenu.setSelectedId(1);
    
    //Oscilloscope
    addAndMakeVisible(scope);
    
    //Look and Feel
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(58, 58, 58));
    getLookAndFeel().setColour(juce::ComboBox::buttonColourId, juce::Colour::fromRGB(58, 58, 58));
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(58, 58, 58));
    getLookAndFeel().setColour(juce::Slider::backgroundColourId, juce::Colour::fromRGB(217, 217, 217));
    getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(217, 217, 217));
    getLookAndFeel().setColour(juce::Slider::trackColourId, juce::Colour::fromRGB(122, 122, 122));
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(122, 122, 122));
    getLookAndFeel().setColour(juce::BubbleComponent::backgroundColourId, juce::Colour::fromRGB(58, 58, 58));
    
    //Essentials I guess
    setSize (700, 200);
    
    scope.setSamplesPerBlock(2);
    startTimerHz(60);
}

MysynthpracAudioProcessorEditor::~MysynthpracAudioProcessorEditor()
{
}

//==============================================================================
void MysynthpracAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour::fromRGB(26, 26, 26));

    juce::Rectangle<int> adsrBox(550, 19, 138, 164);
    g.setColour(juce::Colour::fromRGB(38, 38, 38));
    g.fillRect(adsrBox);

    juce::Rectangle<int> wavetableBox(16, 19, 138, 164);

    g.fillRect(wavetableBox);

  
  
}

void MysynthpracAudioProcessorEditor::resized()
{
    //auto bounds = getLocalBounds();

    waveTypeMenu.setBounds(20, 27, 125, 17);

    ladderButton.setBounds(22, 49, 15, 15);
    ladderButton.changeWidthToFitText();
    ladderModeMenu.setBounds(20, 68, 125, 17);
    
    attackSlider.setBounds(560, 41, 20, 137);
    decaySlider.setBounds(594, 41, 20, 137);
    sustainSlider.setBounds(627, 41, 20, 137);
    releaseSlider.setBounds(660, 41, 20, 137);
    
    masterVolumeSlider.setBounds(184, 135, 335, 20);

    ladderCutOffSlider.setBounds(10, 110, 40, 40);
    ladderResSlider.setBounds(60, 110, 40, 40);
    ladderDriveSlider.setBounds(110, 110, 40, 40);
    
    scope.setBounds(184, 19, 333, 90);
    
}

void MysynthpracAudioProcessorEditor::timerCallback ()
{
    scope.pushBuffer(audioProcessor.cachedBuffer);
    repaint();
    
};

