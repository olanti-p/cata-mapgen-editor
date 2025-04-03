# Bright Nights Mapping Tool

## Known bugs

- On first start, an error shows up about failing to save character template (it's harmless)
- Rendering slows down (UI becomes sluggish) when there are many different graphical tiles to render
- Game sometimes crashes when loading data while minimized/unfocused
- Collapse/expand mappings in palette entry affects all mappings at once
- Undoing symbol change in verbose palette view results in a crash
- The UI may hang if the code tries to display a debug message on start, this can be solved by resizing the game window a couple times
- If fatal error is encontered during data loading, the editor will retry loading instead of returning to welcome screen
- There are no warnings about invalid configuration/missing ids when exporting projects
- Autosave doesn't always handle repeated edits
