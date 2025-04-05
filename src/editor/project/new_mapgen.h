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
    UUID palette;
    OterMapgenBase oter_base = OterMapgenBase::Rows;
    std::string name;

    bool confirmed = false;
    bool cancelled = false;
};

bool show_new_mapgen_window( State &state, NewMapgenState &mapgen );
void add_mapgen( State &state, NewMapgenState &mapgen );

} // namespace editor

#endif // CATA_SRC_EDITOR_NEW_MAPGEN_H
