/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MysynthpracAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    MysynthpracAudioProcessorEditor (MysynthpracAudioProcessor&);
    ~MysynthpracAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback ()override;

    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MysynthpracAudioProcessor& audioProcessor;

    juce::ComboBox waveTypeMenu, ladderModeMenu;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment waveTypeMenuAttatchment, ladderModeMenuAttatchment;

    juce::TextButton ladderButton {"Filer Off!"};
    juce::AudioProcessorValueTreeState::ButtonAttachment ladderButtonAttatchment;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, masterVolumeSlider, ladderCutOffSlider, ladderDriveSlider, ladderResSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttatchment, decayAttatchment, sustainAttatchment, releaseAttatchment, masterVolumeAttatchment, ladderCutOffAttatchment, ladderDriveAttatchment, ladderResAttatchment;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, ladderCuttOffLabel, ladderResonanceLabel, ladderDriveLabel, volumeLabel;
    
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache  { 10 };
    juce::AudioVisualiserComponent scope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MysynthpracAudioProcessorEditor)
};
