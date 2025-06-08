#include "SetPresetNameWindow.h"

SetPresetNameWindow::SetPresetNameWindow()
{
    nameTextEditor = std::make_unique<juce::TextEditor>("nameTextEditor");
    addAndMakeVisible(nameTextEditor.get());
    nameTextEditor->setMultiLine(false);
    nameTextEditor->setReturnKeyStartsNewLine(false);
    nameTextEditor->setReadOnly(false);
    nameTextEditor->setScrollbarsShown(true);
    nameTextEditor->setCaretVisible(true);
    nameTextEditor->setPopupMenuEnabled(false);
    nameTextEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    nameTextEditor->setColour(juce::CaretComponent::caretColourId, juce::Colours::white);
    nameTextEditor->setText(juce::String());

    cancel = std::make_unique<juce::TextButton>("cancel");
    addAndMakeVisible(cancel.get());
    cancel->setButtonText(TRANS("Cancel"));
    cancel->addListener(this);
    cancel->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    cancel->setColour(juce::ComboBox::outlineColourId, juce::Colours::white);

    Ok = std::make_unique<juce::TextButton>("Ok");
    addAndMakeVisible(Ok.get());
    Ok->setButtonText(TRANS("OK"));
    Ok->addListener(this);
    Ok->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    Ok->setColour(juce::ComboBox::outlineColourId, juce::Colours::white);

    setSize(300, 150);
}

SetPresetNameWindow::~SetPresetNameWindow() = default;

void SetPresetNameWindow::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colour(0xff666666));
    g.drawRect(getLocalBounds(), 1);

    g.setColour(juce::Colours::white);
    g.setFont(
        juce::Font(juce::FontOptions().withName("Arial").withStyle("Regular").withHeight(15.0f)));

    g.drawText(TRANS("Preset Name"), 0, proportionOfHeight(0.1f), proportionOfWidth(1.0f),
               proportionOfHeight(0.2f), juce::Justification::centred, true);
}

void SetPresetNameWindow::resized()
{
    nameTextEditor->setBounds(proportionOfWidth(0.15f), proportionOfHeight(0.347f),
                              proportionOfWidth(0.70f), proportionOfHeight(0.173f));

    cancel->setBounds(proportionOfWidth(0.20f), proportionOfHeight(0.70f), proportionOfWidth(0.25f),
                      proportionOfHeight(0.16f));

    Ok->setBounds(proportionOfWidth(0.55f), proportionOfHeight(0.70f), proportionOfWidth(0.25f),
                  proportionOfHeight(0.16f));
}

void SetPresetNameWindow::buttonClicked(juce::Button *buttonThatWasClicked)
{
    if (buttonThatWasClicked == cancel.get())
    {
        callback(0, nameTextEditor->getText());
    }
    else if (buttonThatWasClicked == Ok.get())
    {
        callback(1, nameTextEditor->getText());
    }
}

void SetPresetNameWindow::grabTextEditorFocus() const { nameTextEditor->grabKeyboardFocus(); }