# TODO list

Vague list of what changes are planned, and what changes would be nice to have but are not guaranteed.


## UX
- Easier copy/paste operations, better os clipboard support
- Make all ids selectable
- Better automatic window sizing
- Better palette and mapgen import selection dialogue
- Automatic symbol selection for new terrain/furniture (base on existing symbol usage stats in palettes?)
- "Test in game" button that will hide the editor, spawn the map in game and teleport pc there


## Features
- Configure mod list on start
- Propagate palette symbol change into mapgen & related palettes
- Quick test of mapgen in game (press button -> be spawned in game next to your building)
- Implement all mapgen pieces
- Blueprint autocalc
- Item group editor & tester (in addition to Loot Designer)
- Better vehicle placement preview (graphical tiles; choose which vehicle is visualized)
- Better nested placement preview (recursive placement; graphical tiles and tooltip data; choose which nested is visualized)
- Design simple overmap specials that use the mapgen (like in Hostile Architect)
- Flip/mirror/rotate tool


## UI
- "Discard changes?" when closing game window with Alt+F4 (right now only asks when closing project with `Ctrl+Q`)
- Draw & manipulate mapgenobjects and set functions with mouse
- Localization (may be hard to deal with text layout and rendering)
- Hotkey customization
- Interface colors customization
- Reimplement symbol color mapping


## Technical
- Switch to command-esque pattern for undo/redo
- Figure out if it's possible to have *all* palettes and/or *all* mapgens loaded
- Try to get parametric and mapgen values to work properly
- Better caching for resolved palettes
- Canvas caching to avoid lag in larger mapgens


## Bugs
See BUGS.md


## Not Planned
Things definitely not planned:

- Replicating full mapgen functionality and handling all corner cases
