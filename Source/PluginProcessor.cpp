/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MysynthpracAudioProcessor::MysynthpracAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  ),state(*this, nullptr, "parameters", { std::make_unique<juce::AudioParameterChoice>("wavetype", "WaveType", juce::StringArray{"SINE", "TRIANGLE", "SAW", "SQUARE"}, 0),
                                                        std::make_unique<juce::AudioParameterBool>("ladderbutton", "LadderButton", false),
                                                        std::make_unique<juce::AudioParameterChoice>("laddermode", "LadderMode", juce::StringArray{"LPF12", "HPF12", "BPF12", "LPF24", "HPF24", "BPF24"}, 0),
                                                        std::make_unique<juce::AudioParameterFloat>("attack","Attack",juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f),
                                                        std::make_unique<juce::AudioParameterFloat>("decay","Decay",juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 0.1f),
                                                        std::make_unique<juce::AudioParameterFloat>("sustain","Sustain",juce::NormalisableRange<float> { 0.1f, 1.0f, 0.1f }, 1.0f),
                                                        std::make_unique<juce::AudioParameterFloat>("release","Release",juce::NormalisableRange<float> { 0.1f, 3.0f, 0.1f }, 0.1f),
                                                        std::make_unique<juce::AudioParameterFloat>("volume", "Volume", juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f),
                                                        std::make_unique<juce::AudioParameterFloat>("laddercutoff", "LadderCutOff",juce::NormalisableRange<float>(1.0f, 10000.0f,1),200.0f),
                                                        std::make_unique<juce::AudioParameterFloat>("ladderresonance", "LadderResonance", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f),
                                                        std::make_unique<juce::AudioParameterFloat>("ladderdrive", "LadderDrive", 1.0f, 5.0f, 1.0f)
})
#endif


{
    
    for (auto i = 0; i < 127; ++i)synth.addVoice(new SineWaveVoice());
    synth.addSound(new SineWaveSound());
    //filter.setEnabled(true);
    //setLadderFilter();
}

MysynthpracAudioProcessor::~MysynthpracAudioProcessor()
{
}


//==============================================================================
const juce::String MysynthpracAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MysynthpracAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MysynthpracAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MysynthpracAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MysynthpracAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MysynthpracAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MysynthpracAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MysynthpracAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MysynthpracAudioProcessor::getProgramName (int index)
{
    return {};
}

void MysynthpracAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MysynthpracAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth.setCurrentPlaybackSampleRate(sampleRate);
    
    
    cachedBuffer.setSize(1, samplesPerBlock);
    
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 2;
    spec.maximumBlockSize = samplesPerBlock;
    
    filter.prepare(spec);
}

void MysynthpracAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    //synthAudioSource.releaseResources();
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MysynthpracAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void MysynthpracAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto releaseParam = state.getParameter("release");
    
    auto attack = state.getParameter("attack")->getValue();
    auto decay = state.getParameter("decay")->getValue();
    auto sustain = state.getParameter("sustain")->getValue();
    auto release = releaseParam->convertFrom0to1(releaseParam->getValue());
    
    auto masterVolume = state.getParameter("volume")->getValue();

    int n = (int)*state.getRawParameterValue("wavetype");
    //DBG(n);
    //std::cout << n << std::endl;
    
    auto type = static_cast<SineWaveVoice::WaveType>(n);
    //DBG(type);
    //std::cout << type << std::endl;
    
    
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SineWaveVoice*>(synth.getVoice(i)))
        {
            //std::cout << n << std::endl;
            //DBG(type);
            voice->setWaveType(type);
            voice->setVolume(masterVolume);
            voice->setADSRParameters(attack, decay, sustain, release);
            //DBG(voice.);
        }
    }


    
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> block{buffer};
    setLadderFilter();
    filter.process(juce::dsp::ProcessContextReplacing<float>(block));
    //DBG((int)*state.getRawParameterValue("ladderbutton"));
    
    cachedBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
}

//==============================================================================
bool MysynthpracAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MysynthpracAudioProcessor::createEditor()
{
    return new MysynthpracAudioProcessorEditor (*this);
}

//==============================================================================
void MysynthpracAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MysynthpracAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MysynthpracAudioProcessor();
}

void MysynthpracAudioProcessor::setLadderFilter()
{
    switch ((int)*state.getRawParameterValue("ladderbutton"))
    {
    case 0:
        filter.setEnabled(false);
        break;
    case 1:
        filter.setEnabled(true);
        break;
    default:
        filter.setEnabled(false);
        break;
    }

    switch ((int)*state.getRawParameterValue("laddermode"))
    {
    case 0:
        filter.setMode(juce::dsp::LadderFilterMode::LPF12);
        break;
    case 1:
        filter.setMode(juce::dsp::LadderFilterMode::HPF12);
        break;
    case 2:
        filter.setMode(juce::dsp::LadderFilterMode::BPF12);
        break;
    case 3:
        filter.setMode(juce::dsp::LadderFilterMode::LPF24);
        break;
    case 4:
        filter.setMode(juce::dsp::LadderFilterMode::HPF24);
        break;
    case 5:
        filter.setMode(juce::dsp::LadderFilterMode::BPF24);
        break;
    default:
        filter.setMode(juce::dsp::LadderFilterMode::LPF12);
        break;
    }
    auto cutoffparam =state.getParameter("laddercutoff");
    auto driveParam = state.getParameter("ladderdrive");
    
    filter.setCutoffFrequencyHz(cutoffparam->convertFrom0to1(cutoffparam->getValue()));
    filter.setResonance(state.getParameter("ladderresonance")->getValue());
    filter.setDrive(driveParam->convertFrom0to1(driveParam->getValue()));
   
}
