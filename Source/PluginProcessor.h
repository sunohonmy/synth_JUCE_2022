/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>



//==============================================================================
/**
 */

class WavetableOscillator
{
public:
    WavetableOscillator (juce::AudioSampleBuffer& wavetableToUse)
    : wavetable (wavetableToUse),
    tableSize (wavetable.getNumSamples() - 1)
    {
        jassert (wavetable.getNumChannels() == 1);
    }
    
    void setFrequency (float frequency, float sampleRate)
    {
        auto tableSizeOverSampleRate = (float) tableSize / sampleRate;
        tableDelta = frequency * tableSizeOverSampleRate;
    }
    
    forcedinline float getNextSample() noexcept
    {
        
        auto index0 = (unsigned int) currentIndex;
        auto index1 = index0 + 1;
        
        auto frac = currentIndex - (float) index0;
        
        auto* table = wavetable.getReadPointer (0);
        auto value0 = table[index0];
        auto value1 = table[index1];
        
        auto currentSample = value0 + frac * (value1 - value0);
        
        if ((currentIndex += tableDelta) > (float) tableSize)
            currentIndex -= (float) tableSize;
        
        return currentSample;
    }
    
    void setWavetable(const juce::AudioSampleBuffer& wavetableToUse)
    {
        jassert (wavetableToUse.getNumChannels() == 1);
        
        wavetable = wavetableToUse;
        tableSize = wavetable.getNumSamples() - 1;
        
    }
    
    juce::AudioSampleBuffer& getWavetable()
    {
        return wavetable;
    }
    
private:
    juce::AudioSampleBuffer& wavetable;
    int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f;
};

class SineWaveSound : public juce::SynthesiserSound
{
public:
    SineWaveSound() {}
    
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SineWaveVoice : public juce::SynthesiserVoice
{
public:
    enum WaveType {
        SINE = 0,
        TRIANGLE,
        SAW,
        SQUARE
    };
private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;
    
    const unsigned int tableSize = 1 << 7;
    float masterVolume;
    
    juce::AudioSampleBuffer sineTable, sawTable, triTable, squareTable;
    juce::OwnedArray<WavetableOscillator> oscillators;
    
    WaveType waveType;
    
    std::unique_ptr<WavetableOscillator> oscillator;
    //auto osc : oscillators;
    
public:
    
    SineWaveVoice()
    {
        createTriWavetable();
        createSineWavetable();
        createSawWavetable();
        createSquareWavetable();
        
        oscillator = std::make_unique<WavetableOscillator>(sineTable);
        
        
      
        setWaveType(waveType);
    }
    void setWaveType(WaveType type, bool disableCheck = false)
    {
        if(!disableCheck)
            if(type == waveType)return;
        
        
        switch(type){
            case(SINE):
                oscillator->setWavetable(sineTable);
                
                break;
            case(TRIANGLE):
                oscillator->setWavetable(triTable);
                
                break;
            case(SAW):
                oscillator->setWavetable(sawTable);
                
                break;
            case(SQUARE):
                oscillator->setWavetable(squareTable);
                
                break;
            default :
                oscillator->setWavetable(sineTable);
                
                break;
        }
        
        waveType = type;
    }
    
    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*>(sound) != nullptr;
    }
    
    void createSineWavetable()
    {
        sineTable.setSize (1, (int) tableSize + 1);
        sineTable.clear();
        
        auto* samples = sineTable.getWritePointer (0);
        
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize);
        auto currentAngle = 0.0;
        for(unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = std::sin(currentAngle);
            samples[i] += (float) sample;
            currentAngle += angleDelta;
        }
        samples[tableSize] = samples[0];
        
        
    }
    
    void createSawWavetable()
    {
        sawTable.setSize(1, (int)tableSize + 1);
        sawTable.clear();
        
        auto* samples = sawTable.getWritePointer(0);
        
        auto angleDelta = 1.0 / (double)(tableSize);
        auto currentAngle = 0.0;
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = 2.0 * (currentAngle - floor(0.5 + currentAngle));
            samples[i] += (float)sample;
            currentAngle += angleDelta;
        }
        samples[tableSize] = samples[0];
        
        
    }
    
    void createSquareWavetable()
    {
        squareTable.setSize(1, (int)tableSize + 1);
        squareTable.clear();
        
        auto* samples = squareTable.getWritePointer(0);
        
        auto angleDelta = 1.0 / (double)(tableSize);
        auto currentAngle = 0.0;
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = 2.0 * (2.0 * floor(currentAngle) - floor(2.0 * currentAngle)) + 1.0;
            samples[i] += (float)sample;
            currentAngle += angleDelta;
        }
        samples[tableSize] = samples[0];
        
        
    }
    
    void createTriWavetable()
    {
        triTable.setSize(1, (int)tableSize + 1);
        triTable.clear();
        
        auto* samples = triTable.getWritePointer(0);
        
        auto angleDelta = 1.0 / (double)(tableSize);
        auto currentAngle = 0.0;
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = 4.0 * abs(currentAngle - floor(currentAngle + 0.75) + 0.25) - 1.0;
            samples[i] += (float)sample;
            currentAngle += angleDelta;
        }
        samples[tableSize] = samples[0];
        
        
    }
    
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        auto sampleRate = getSampleRate();
        
        setWaveType(waveType,true);
        
        auto frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        
        oscillator->setFrequency((float)frequency, (float)sampleRate);
        
        adsr.noteOn();
    }
    
    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        adsr.noteOff();
        
        if (!allowTailOff || !adsr.isActive())
        {
            clearCurrentNote();
        }
    }
    
    void prepareToPlay(double sampleRate)
    {
        reset();
        
        sampleRate = getSampleRate();
        adsr.setSampleRate(sampleRate);
    }
    
    void setVolume(float theMasterVolume)
    {
        masterVolume = theMasterVolume;
    }
    
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        
        while(--numSamples >= 0)
        {
            auto oscillatorOutput = 0.0f;
            
            oscillatorOutput+= oscillator->getNextSample();
            auto currentSample = oscillatorOutput * masterVolume * adsr.getNextSample();
            
            
            for( auto i = outputBuffer.getNumChannels(); --i >= 0;)outputBuffer.addSample(i, startSample, currentSample);
            ++startSample;
        }

        
    }
    
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    
    void reset()
    {
        adsr.reset();
    }
    
    void setADSRParameters(const float attack, const  float decay, const float sustain, const float release)
    {
        adsrParameters.attack = attack;
        adsrParameters.decay = decay;
        adsrParameters.sustain = sustain;
        adsrParameters.release = release;
        
        adsr.setParameters(adsrParameters);
    }
    
};

class SynthAudioSource  : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState)
    : keyboardState(keyState)
    {
        for (auto i = 0; i < 10; ++i)synth.addVoice(new SineWaveVoice());
        synth.addSound(new SineWaveSound());
    }
    
    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }
    
    void prepareToPlay(int /*samplesPerBlock*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        midiCollector.reset(sampleRate);
        
    }
    
    void releaseResources() override{}
    
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        
        
        juce::MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
        
        keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
        
        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }
    
    juce::MidiMessageCollector* getMidiCollector()
    {
        return &midiCollector;
    }
    
private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    
    juce::MidiMessageCollector midiCollector;
    
};

class MysynthpracAudioProcessor  : public juce::AudioProcessor, private juce::Timer
#if JucePlugin_Enable_ARA
, public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    MysynthpracAudioProcessor();
    ~MysynthpracAudioProcessor() override;
    
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
    
    juce::AudioProcessorValueTreeState state;
    juce::AudioBuffer<float> cachedBuffer;

    void setLadderFilter();

private:
    //==============================================================================
    juce::MidiKeyboardState keyboardState;
    //SynthAudioSource synthAudioSource;
    //juce::MidiKeyboardComponent keyboardComponent;
    juce::Synthesiser synth;
    
    juce::dsp::LadderFilter<float> filter;
    
    //MIDI inputs
    juce::ComboBox midiInputList;
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;
    
    void timerCallback() override
    {
        //keyboardComponent.grabKeyboardFocus();
        stopTimer();
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MysynthpracAudioProcessor)
};
