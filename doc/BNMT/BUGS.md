# Known bugs

- On first start, an error shows up about failing to save character template (it's harmless, create templates folder to silence it)
- Editor does not handle all mapgen features, so importing and then exporting may incur data loss or unexpected changes
- Many place_* functions and symbol mappings are not implemented, or implemented only partially
- Vehicle placement outline preview may not account for fake parts
- Nested placement outline does not account for nested-inside-nested, or vehicle-inside-nested
- Rendering slows down (UI becomes sluggish) when there are many different graphical tiles to render
- Game sometimes crashes when loading data while minimized/unfocused
- Game sometimes crashes on exit
- The UI may hang if the code tries to display a debug message on start, this can be solved by resizing the game window a couple times
- If fatal error is encontered during data loading, the editor will retry loading instead of returning to welcome screen
- There are no warnings about invalid configuration/missing ids when exporting projects
- Autosave doesn't always recognize repeated edits as changes worthy of saving
- Oversize sprites are not rendered properly
- Will crash if palette or mapgen rows matrix contains unicode symbol with combining characters (there are like 1 or 2 cases of this in vanilla)
