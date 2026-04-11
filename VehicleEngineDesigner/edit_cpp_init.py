filepath = r'D:\JUCE\zazzVSTPlugins\VehicleEngineDesigner\Source\MainComponent.cpp'
with open(filepath, 'r', encoding='utf-8') as f:
    content = f.read()

# Add DC filter checkbox initialization after m_displayModeButton initialization
old_display_button = '''\taddAndMakeVisible(&m_displayModeButton);
\tm_displayModeButton.setButtonText("Waveform");
\tm_displayModeButton.onClick = [this] { displayModeButtonClicked(); };

\t// Combo boxes'''

new_display_button = '''\taddAndMakeVisible(&m_displayModeButton);
\tm_displayModeButton.setButtonText("Waveform");
\tm_displayModeButton.onClick = [this] { displayModeButtonClicked(); };

\taddAndMakeVisible(&m_applyDCFilterCheckbox);
\tm_applyDCFilterCheckbox.setButtonText("Apply DC Filter");
\tm_applyDCFilterCheckbox.setToggleState(m_applyDCFilter, juce::dontSendNotification);
\tm_applyDCFilterCheckbox.onClick = [this] {
\t\tm_applyDCFilter = m_applyDCFilterCheckbox.getToggleState();
\t};

\taddAndMakeVisible(&m_applyDCFilterLabel);
\tm_applyDCFilterLabel.setText("DC Filter", juce::dontSendNotification);
\tm_applyDCFilterLabel.attachToComponent(&m_applyDCFilterCheckbox, true);

\t// Combo boxes'''

if old_display_button in content:
    content = content.replace(old_display_button, new_display_button)
    print("✓ Added DC filter checkbox initialization")
else:
    print("✗ Display button section not found")

# Write back
with open(filepath, 'w', encoding='utf-8') as f:
    f.write(content)

print("✓ cpp file updated successfully")
