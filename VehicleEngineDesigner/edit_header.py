filepath = r'D:\JUCE\zazzVSTPlugins\VehicleEngineDesigner\Source\MainComponent.h'
with open(filepath, 'r', encoding='utf-8') as f:
    content = f.read()

# First replacement: Add checkbox and label members after m_displayModeButton
old_button_section = '\tjuce::TextButton m_displayModeButton;\n\n\t// Sliders'
new_button_section = '\tjuce::TextButton m_displayModeButton;\n\n\tjuce::ToggleButton m_applyDCFilterCheckbox;\n\tjuce::Label m_applyDCFilterLabel;\n\n\t// Sliders'

if old_button_section in content:
    content = content.replace(old_button_section, new_button_section)
    print("✓ Added checkbox and label members")
else:
    print("✗ Button section not found")

# Second replacement: Add bool flag in state variables
old_state = '\tbool m_showSpectrogram = false;\n\tDisplayMode m_displayMode = DisplayMode::Waveform;'
new_state = '\tbool m_showSpectrogram = false;\n\tbool m_applyDCFilter = false;\n\tDisplayMode m_displayMode = DisplayMode::Waveform;'

if old_state in content:
    content = content.replace(old_state, new_state)
    print("✓ Added m_applyDCFilter state flag")
else:
    print("✗ State section not found")

# Write back
with open(filepath, 'w', encoding='utf-8') as f:
    f.write(content)

print("✓ File updated successfully")
