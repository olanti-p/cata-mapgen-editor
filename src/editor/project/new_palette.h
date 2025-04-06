#ifndef CATA_SRC_EDITOR_NEW_PALETTE_H
#define CATA_SRC_EDITOR_NEW_PALETTE_H

#include "common/uuid.h"
#include "widget/editable_id.h"

#include <string>

namespace editor
{
struct Palette;
struct Project;
struct State;

struct NewPaletteState {
    bool import_temp;
    EID::Palette import_from;
    EID::TempPalette import_temp_from;

    bool confirmed = false;
    bool cancelled = false;
};

// FIXME: remove palette creation code from this and leave only import
bool show_new_palette_window( State &state, NewPaletteState &palette );

} // namespace editor

#endif // CATA_SRC_EDITOR_NEW_PALETTE_H
