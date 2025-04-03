#ifndef CATA_SRC_EDITOR_PALETTE_MAKING_H
#define CATA_SRC_EDITOR_PALETTE_MAKING_H

#include "palette.h"

namespace editor
{
struct Mapgen;
struct Project;

map_key pick_available_key( const Palette &pal );

PaletteEntry make_simple_entry( Project &project, Palette &palette, Mapping &&mapping );
Mapping make_mapping( const EID::Ter *ter, const EID::Furn *furn );

struct PaletteImportReport {
    int num_total = 0;
    int num_failed = 0;
};

PaletteImportReport import_palette_data( Project &project, Palette &palette,
        const EID::Palette &source_id );

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_MAKING_H
