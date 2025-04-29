#ifndef CATA_SRC_EDITOR_NEW_MAPGEN_H
#define CATA_SRC_EDITOR_NEW_MAPGEN_H

#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "common/uuid.h"

namespace editor
{
struct State;

struct NewMapgenState {
    MapgenType mtype = MapgenType::Oter;
    UUID palette = UUID_INVALID;
    std::string name;

    bool confirmed = false;
    bool cancelled = false;
};

struct ImportMapgenState {
    MapgenType mtype = MapgenType::Oter;
    std::string oter;
    std::string nested;
    std::string update;

    bool confirmed = false;
    bool cancelled = false;
};

bool show_new_mapgen_window( State &state, NewMapgenState &mapgen );
bool show_import_mapgen_window(State& state, ImportMapgenState& mapgen);
void add_mapgen( State &state, NewMapgenState &mapgen );
Mapgen* import_mapgen( State &state, ImportMapgenState &mapgen );

void list_nests_from_piece(Project& project, std::set<std::string>& list, Piece& piece);
void list_nests_from_palette(Project& project, std::set<std::string>& list, Palette& palette);
void list_nests_from_mapgen(Project& project, std::set<std::string>& list, Mapgen& mapgen);
void quick_import_all_nests(State& state, Mapgen& mapgen);

} // namespace editor

#endif // CATA_SRC_EDITOR_NEW_MAPGEN_H
