#ifndef CATA_SRC_EDITOR_PALETTE_MAKING_H
#define CATA_SRC_EDITOR_PALETTE_MAKING_H

#include "palette.h"
#include "palette_import_report.h"

namespace editor
{
struct Mapgen;
struct Project;

map_key pick_available_key( const Palette &pal );

PaletteEntry make_simple_entry( Project &project, Palette &palette, Mapping &&mapping );
Mapping make_mapping( const EID::Ter *ter, const EID::Furn *furn );

PaletteImportReport import_palette_data( Project &project, Palette &palette,
        const EID::Palette &source_id );

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_MAKING_H
