#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DiscombobulatorAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Button::Listener,
    private juce::TextEditor::Listener
{
public:
    DiscombobulatorAudioProcessorEditor(DiscombobulatorAudioProcessor&);
    ~DiscombobulatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked(juce::Button* button) override;
    void textEditorTextChanged(juce::TextEditor&) override;

    void loadAudioFile();
    void scrambleAndSave();
    void descrambleAndSave();

    DiscombobulatorAudioProcessor& audioProcessor;

    juce::TextEditor passkeyEditor;
    juce::Label passkeyLabel;

    juce::TextButton loadButton;
    juce::TextButton scrambleButton;
    juce::TextButton descrambleButton;

    juce::Label statusLabel;
    juce::Label infoLabel;

    juce::AudioBuffer<float> loadedAudio;
    double loadedSampleRate = 44100.0;
    bool audioLoaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiscombobulatorAudioProcessorEditor)
};