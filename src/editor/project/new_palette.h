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

enum class NewPaletteKind {
    BrandNew,
    Imported,
};

struct NewPaletteState {
    std::string name;
    NewPaletteKind kind = NewPaletteKind::Imported;
    EID::Palette import_from;
    bool inherits = false;
    UUID inherits_from = UUID_INVALID;

    bool confirmed = false;
    bool cancelled = false;
};

// FIXME: remove palette creation code from this and leave only import
bool show_new_palette_window( State &state, NewPaletteState &palette );
void add_palette( State &state, NewPaletteState &palette );

} // namespace editor

#endif // CATA_SRC_EDITOR_NEW_PALETTE_H
