#ifndef CATA_SRC_EDITOR_PALETTE_VIEW_H
#define CATA_SRC_EDITOR_PALETTE_VIEW_H

#include "common/uuid.h"

namespace editor
{
struct State;
struct Mapgen;
struct Project;
struct Palette;
struct PaletteEntry;

struct PaletteEntryDragState {
    UUID palette = UUID_INVALID;
    UUID entry = UUID_INVALID;
};

void show_mapping( State &state, Palette &p, PaletteEntry &entry, bool &show );
void show_palette_verbose( State &state, Palette &p, bool &show );
void show_palette_simple( State &state, Palette &p, bool &show );
void show_palette_entry_tooltip( const PaletteEntry &entry );
bool handle_palette_entry_drag_and_drop( Project &project, Palette &palette, int idx );

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_VIEW_H
