#ifndef CATA_SRC_EDITOR_PALETTE_WINDOW_H
#define CATA_SRC_EDITOR_PALETTE_WINDOW_H

#include "common/map_key.h"
#include "common/uuid.h"

namespace editor
{
struct State;
struct Mapgen;
struct Project;
struct Palette;
struct PaletteEntry;
struct ViewEntry;
struct ViewPalette;

struct PaletteEntryDragState {
    UUID palette = UUID_INVALID;
    MapKey entry;
};

void show_mapping_source( State &state, Palette &p, PaletteEntry &entry, bool &show );
void show_mapping_resolved(State& state, ViewPalette& p, ViewEntry& entry, bool& show);
void show_active_palette_details( State &state, Palette &p, bool &show );
void show_active_palette_simple( State &state, Palette &p, bool &show, bool resolved );
void show_palette_preview(State& state, Palette& p, bool& show );
void show_palette_entries_simple( State& state, Palette& palette );
void show_palette_entries_simple(State& state, ViewPalette& palette);
void show_palette_entry_tooltip( const PaletteEntry &entry );
void show_palette_entry_tooltip(const Project& project, const ViewEntry& entry );
bool handle_palette_entry_drag_and_drop( Project &project, Palette &palette, int idx );

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_WINDOW_H
