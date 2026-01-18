#include "PluginProcessor.h"
#include "PluginEditor.h"

DiscombobulatorAudioProcessor::DiscombobulatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    passkey = "00000000"; // Default passkey
}

DiscombobulatorAudioProcessor::~DiscombobulatorAudioProcessor()
{
}

const juce::String DiscombobulatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DiscombobulatorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool DiscombobulatorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool DiscombobulatorAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double DiscombobulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DiscombobulatorAudioProcessor::getNumPrograms()
{
    return 1;
}

int DiscombobulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DiscombobulatorAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String DiscombobulatorAudioProcessor::getProgramName(int index)
{
    return {};
}

void DiscombobulatorAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void DiscombobulatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void DiscombobulatorAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DiscombobulatorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void DiscombobulatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Pass through in real-time - scrambling happens offline
}

bool DiscombobulatorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* DiscombobulatorAudioProcessor::createEditor()
{
    return new DiscombobulatorAudioProcessorEditor(*this);
}

void DiscombobulatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(passkey);
}

void DiscombobulatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    passkey = stream.readString();
}

// Custom scrambling methods
void DiscombobulatorAudioProcessor::setPasskey(const juce::String& key)
{
    passkey = key;
}

std::vector<int> DiscombobulatorAudioProcessor::generateShufflePattern(int numChunks, unsigned int seed)
{
    std::vector<int> indices;
    for (int i = 0; i < numChunks; ++i)
        indices.push_back(i);

    std::mt19937 rng(seed);
    std::shuffle(indices.begin(), indices.end(), rng);

    return indices;
}

void DiscombobulatorAudioProcessor::scrambleAudio(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    int numChunks = (numSamples + CHUNK_SIZE - 1) / CHUNK_SIZE;

    // Convert passkey to seed
    unsigned int seed = 0;
    for (int i = 0; i < passkey.length(); ++i)
        seed = seed * 31 + (unsigned int)passkey[i];

    auto shufflePattern = generateShufflePattern(numChunks, seed);

    // Create temporary buffer
    juce::AudioBuffer<float> tempBuffer(numChannels, numSamples);
    tempBuffer.clear();

    // Scramble chunks
    for (int chunk = 0; chunk < numChunks; ++chunk)
    {
        int srcStart = chunk * CHUNK_SIZE;
        int dstStart = shufflePattern[chunk] * CHUNK_SIZE;
        int chunkLen = juce::jmin(CHUNK_SIZE, numSamples - srcStart);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (dstStart + chunkLen <= numSamples)
            {
                tempBuffer.copyFrom(ch, dstStart, buffer, ch, srcStart, chunkLen);
            }
        }
    }

    // Copy back
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom(ch, 0, tempBuffer, ch, 0, numSamples);
}

void DiscombobulatorAudioProcessor::descrambleAudio(juce::AudioBuffer<float>& buffer)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    int numChunks = (numSamples + CHUNK_SIZE - 1) / CHUNK_SIZE;

    // Convert passkey to seed
    unsigned int seed = 0;
    for (int i = 0; i < passkey.length(); ++i)
        seed = seed * 31 + (unsigned int)passkey[i];

    auto shufflePattern = generateShufflePattern(numChunks, seed);

    // Create temporary buffer
    juce::AudioBuffer<float> tempBuffer(numChannels, numSamples);
    tempBuffer.clear();

    // Reverse the scramble - read from shuffled positions, write to original
    for (int chunk = 0; chunk < numChunks; ++chunk)
    {
        int srcStart = shufflePattern[chunk] * CHUNK_SIZE;
        int dstStart = chunk * CHUNK_SIZE;
        int chunkLen = juce::jmin(CHUNK_SIZE, numSamples - dstStart);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (srcStart + chunkLen <= numSamples)
            {
                tempBuffer.copyFrom(ch, dstStart, buffer, ch, srcStart, chunkLen);
            }
        }
    }

    // Copy back
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom(ch, 0, tempBuffer, ch, 0, numSamples);
}

bool DiscombobulatorAudioProcessor::saveScrambledWav(const juce::File& file, juce::AudioBuffer<float>& buffer, double sampleRate)
{
    if (file.exists())
        file.deleteFile();

    std::unique_ptr<juce::FileOutputStream> outputStream(file.createOutputStream());

    if (outputStream == nullptr)
        return false;

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer;

    writer.reset(wavFormat.createWriterFor(outputStream.get(),
        sampleRate,
        buffer.getNumChannels(),
        16,
        {},
        0));

    if (writer != nullptr)
    {
        outputStream.release(); // Writer takes ownership
        writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
        return true;
    }

    return false;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DiscombobulatorAudioProcessor();
}