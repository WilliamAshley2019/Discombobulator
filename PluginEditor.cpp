#include "PluginProcessor.h"
#include "PluginEditor.h"

DiscombobulatorAudioProcessorEditor::DiscombobulatorAudioProcessorEditor(DiscombobulatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 300);

    // Passkey editor
    passkeyLabel.setText("Passkey (8 digits):", juce::dontSendNotification);
    passkeyLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(passkeyLabel);

    passkeyEditor.setText(audioProcessor.getPasskey());
    passkeyEditor.setInputRestrictions(8, "0123456789");
    passkeyEditor.setJustification(juce::Justification::centred);
    passkeyEditor.addListener(this);
    addAndMakeVisible(passkeyEditor);

    // Buttons
    loadButton.setButtonText("Load Audio File");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);

    scrambleButton.setButtonText("Scramble & Save");
    scrambleButton.addListener(this);
    scrambleButton.setEnabled(false);
    addAndMakeVisible(scrambleButton);

    descrambleButton.setButtonText("Descramble & Save");
    descrambleButton.addListener(this);
    descrambleButton.setEnabled(false);
    addAndMakeVisible(descrambleButton);

    // Status label
    statusLabel.setText("Load an audio file to begin", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(statusLabel);

    // Info label
    infoLabel.setText("Discombobulator v1.0 - Scramble audio with a passkey", juce::dontSendNotification);
    infoLabel.setJustificationType(juce::Justification::centred);
    infoLabel.setFont(juce::FontOptions(12.0f));
    infoLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(infoLabel);
}

DiscombobulatorAudioProcessorEditor::~DiscombobulatorAudioProcessorEditor()
{
}

void DiscombobulatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("DISCOMBOBULATOR", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);
}

void DiscombobulatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    area.removeFromTop(50); // Title space

    infoLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(10);

    auto passkeyArea = area.removeFromTop(30);
    passkeyLabel.setBounds(passkeyArea.removeFromLeft(150));
    passkeyEditor.setBounds(passkeyArea.removeFromLeft(120));

    area.removeFromTop(20);

    loadButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    scrambleButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    descrambleButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);

    statusLabel.setBounds(area.removeFromTop(30));
}

void DiscombobulatorAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        loadAudioFile();
    }
    else if (button == &scrambleButton)
    {
        scrambleAndSave();
    }
    else if (button == &descrambleButton)
    {
        descrambleAndSave();
    }
}

void DiscombobulatorAudioProcessorEditor::textEditorTextChanged(juce::TextEditor&)
{
    audioProcessor.setPasskey(passkeyEditor.getText());
}

void DiscombobulatorAudioProcessorEditor::loadAudioFile()
{
    auto chooser = std::make_shared<juce::FileChooser>("Select an audio file to load...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.mp3;*.aif;*.aiff");

    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file == juce::File{})
                return;

            juce::AudioFormatManager formatManager;
            formatManager.registerBasicFormats();

            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

            if (reader != nullptr)
            {
                loadedSampleRate = reader->sampleRate;
                loadedAudio.setSize(reader->numChannels, (int)reader->lengthInSamples);
                reader->read(&loadedAudio, 0, (int)reader->lengthInSamples, 0, true, true);

                audioLoaded = true;
                scrambleButton.setEnabled(true);
                descrambleButton.setEnabled(true);

                statusLabel.setText("Audio loaded: " + file.getFileName(), juce::dontSendNotification);
            }
            else
            {
                statusLabel.setText("Error loading file!", juce::dontSendNotification);
            }
        });
}

void DiscombobulatorAudioProcessorEditor::scrambleAndSave()
{
    if (!audioLoaded) return;

    auto chooser = std::make_shared<juce::FileChooser>("Save scrambled audio as...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav");

    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
        {
            auto outputFile = fc.getResult();

            if (outputFile == juce::File{})
                return;

            // Make a copy to scramble
            juce::AudioBuffer<float> bufferCopy(loadedAudio.getNumChannels(), loadedAudio.getNumSamples());
            for (int ch = 0; ch < loadedAudio.getNumChannels(); ++ch)
                bufferCopy.copyFrom(ch, 0, loadedAudio, ch, 0, loadedAudio.getNumSamples());

            audioProcessor.scrambleAudio(bufferCopy);

            if (audioProcessor.saveScrambledWav(outputFile, bufferCopy, loadedSampleRate))
            {
                statusLabel.setText("Scrambled audio saved!", juce::dontSendNotification);
            }
            else
            {
                statusLabel.setText("Error saving file!", juce::dontSendNotification);
            }
        });
}

void DiscombobulatorAudioProcessorEditor::descrambleAndSave()
{
    if (!audioLoaded) return;

    auto chooser = std::make_shared<juce::FileChooser>("Save descrambled audio as...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav");

    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this, chooser](const juce::FileChooser& fc)
        {
            auto outputFile = fc.getResult();

            if (outputFile == juce::File{})
                return;

            // Make a copy to descramble
            juce::AudioBuffer<float> bufferCopy(loadedAudio.getNumChannels(), loadedAudio.getNumSamples());
            for (int ch = 0; ch < loadedAudio.getNumChannels(); ++ch)
                bufferCopy.copyFrom(ch, 0, loadedAudio, ch, 0, loadedAudio.getNumSamples());

            audioProcessor.descrambleAudio(bufferCopy);

            if (audioProcessor.saveScrambledWav(outputFile, bufferCopy, loadedSampleRate))
            {
                statusLabel.setText("Descrambled audio saved!", juce::dontSendNotification);
            }
            else
            {
                statusLabel.setText("Error saving file!", juce::dontSendNotification);
            }
        });
}