# Bright Nights Mapping Tool

These changes would be great to have in the editor.


## UX
- Make "Rows" the default mapgen base
- Make "Brush" the default tool
- Automatically switch palette depending on what mapgen is active (so it's impossible to use wrong palette)
- Allow switching palettes (without migrating tiles?)
- Fill 'empty' tiles with the graphical fill_ter tile (can be toggled with a switch)
- Switch to toggle visibility of symbols on canvas
- Switch to toggle visibility of graphical tiles on canvas


## Features

- Quick test of mapgen in game (press button -> be spawned in game next to your building)
- Implement all mapgen pieces
- Palette inheritance
- Update mapgen, blueprint autocalc
- Item group editor & tester
- Vehicle placement preview (boundaries around placement point)
- Import existing mapgens
- Import from pixels
- Import from Excel/text
- Design simple overmap specials that use the mapgen


## UI
- "Discard changes?" when closing game window with Alt+F4 (right now only asks when closing project with `Ctrl+Q`)
- Draw & manipulate mapgenobjects with mouse
- Show terrain underneath furniture in preview
- Localization (may be hard to deal with text layout and rendering)
- Hotkey customization (have to create new hotkey management system; and what about ImGui hotkeys?)


## Bugs
See BUGS.md
