#ifndef CATA_SRC_EDITOR_PALETTE_MAKING_H
#define CATA_SRC_EDITOR_PALETTE_MAKING_H

#include "palette.h"
#include "palette_import_report.h"

class mapgen_palette;

namespace editor
{
struct Mapgen;
struct Project;

map_key pick_available_key( const Palette &pal );

PaletteEntry make_simple_entry( Project &project, Palette &palette, Mapping &&mapping );
Mapping make_mapping( const EID::Ter *ter, const EID::Furn *furn );

std::vector<std::vector<std::string>>
calc_palette_ancestors(const mapgen_palette& source);

void import_palette_data_and_report(State& state, Palette& destination, EID::Palette source);
void reimport_palette(State& state, Palette& p);
void quick_import_palette(State& state, EID::Palette p);
Palette& quick_create_palette(State& state);

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_MAKING_H
