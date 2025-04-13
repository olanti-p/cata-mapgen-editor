#ifndef CATA_SRC_EDITOR_PALETTE_MAKING_H
#define CATA_SRC_EDITOR_PALETTE_MAKING_H

#include "palette.h"
#include "palette_import_report.h"

class mapgen_palette;

namespace editor
{
struct Mapgen;
struct Project;

MapKey pick_available_key( const Palette &pal );

PaletteEntry make_simple_entry( Project &project, Palette &palette, const EID::Ter* ter, const EID::Furn* furn );

std::vector<std::vector<std::string>>
calc_palette_ancestors(const mapgen_palette& source);

std::unique_ptr<Piece> import_simple_piece(const jmapgen_piece& piece, PaletteImportReport& report, bool is_object);

void import_palette_data_and_report(State& state, Palette& destination, EID::Palette source);
void reimport_palette(State& state, Palette& p);
void recursive_import_palette(State& state, Palette& p);
Palette& quick_import_palette(State& state, EID::Palette p);
void quick_import_temp_palette(State& state, EID::TempPalette p);
Palette& quick_create_palette(State& state);

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_MAKING_H
