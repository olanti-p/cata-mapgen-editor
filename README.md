# Bright Nights Mapping Tool

Map making tool for [Cataclysm: Bright Nights](https://github.com/cataclysmbnteam/Cataclysm-BN).

![readme pic](doc/BNMT/assets/readme_pic.png "readme_pic")

## How to install
See [INSTALL.md](doc/BNMT/INSTALL.md)

## How to compile
See [COMPILING.md](doc/BNMT/COMPILING.md)

## How to use
See [TUTORIAL.md](doc/BNMT/TUTORIAL.md)

## Features
List of currently implemented features.

Features marked with `*` are implemented only partially (some planned functionality is missing, or there may be bugs).

- Creating, saving and loading projects
- Exporting projects into mapgen
- Drawing with mouse
   - Brush (hold mouse button to draw)
   - Pipette (pick tile from canvas)
   - Line drawing tool
   - Rectangle tool
   - Bucket tool (flood fill, global fill, fill in selection)
   - Ruler tool (measure distance and area)
- Copy/cut/paste tiles
- Work on multiple mapgens at once
- Use different palettes for mapgens in single project
- Autocomplete and fuzzy search for identifiers
- Undo/redo support
- UI retains its state between sessions
- Can be used with mods
- *Autosaves
- *Support for big mapgens (e.g. 48x48, 72x72 - any multiple of 24)
- *Import existing palettes from game files
- *Assign different mappings to symbols (terrain/furniture/items/fields/graffiti/vehicle/monster/NPC/other)
- *Mapgen objects (spawning objects within an area)
- *Update-type mapgens
- *Nested-type mapgens
- *QoL UI features (hotkeys, tooltips)

## Bugs and limitations
See [TODO.md](doc/BNMT/TODO.md) for list of planned features.

See [BUGS.md](doc/BNMT/BUGS.md) for list of known bugs.

If you don't see your issue there, please open a ticket or contact me on Discord.

## Feedback
I'm available on official Bright Nights Discord server: https://discord.gg/XW7XhXuZ89 , user id `olanti_p`.

## License
BNMT is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
