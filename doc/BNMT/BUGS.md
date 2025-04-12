# Known bugs

- On first start, an error shows up about failing to save character template (it's harmless)
- Many pieces are not implemented, and show up as Unknown. Exporting imported palettes may lead to data loss.
- Many place_* functions are not implemented, and show up as Unknown. Exporting imported mapgens may lead to data loss.
- Rendering slows down (UI becomes sluggish) when there are many different graphical tiles to render
- Game sometimes crashes when loading data while minimized/unfocused
- The UI may hang if the code tries to display a debug message on start, this can be solved by resizing the game window a couple times
- If fatal error is encontered during data loading, the editor will retry loading instead of returning to welcome screen
- There are no warnings about invalid configuration/missing ids when exporting projects
- Autosave doesn't always recognize repeated edits as changes worthy of saving
