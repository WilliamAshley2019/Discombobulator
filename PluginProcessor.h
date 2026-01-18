#pragma once

#include <JuceHeader.h>
#include <random>

class DiscombobulatorAudioProcessor : public juce::AudioProcessor
{
public:
    DiscombobulatorAudioProcessor();
    ~DiscombobulatorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Custom methods for scrambling
    void setPasskey(const juce::String& key);
    juce::String getPasskey() const { return passkey; }

    void scrambleAudio(juce::AudioBuffer<float>& buffer);
    void descrambleAudio(juce::AudioBuffer<float>& buffer);

    bool saveScrambledWav(const juce::File& file, juce::AudioBuffer<float>& buffer, double sampleRate);

private:
    juce::String passkey;
    static const int CHUNK_SIZE = 512; // Samples per chunk

    // Generate shuffle indices based on passkey
    std::vector<int> generateShufflePattern(int numChunks, unsigned int seed);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiscombobulatorAudioProcessor)
};