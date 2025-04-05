#pragma once
#ifndef CATA_SRC_MAPGEN_SPAWN_DATA_H
#define CATA_SRC_MAPGEN_SPAWN_DATA_H

#include <map>
#include <vector>

#include "type_id.h"
#include "mapgen_int_range.h"
#include "coordinates.h"

struct spawn_data {
    std::map<itype_id, jmapgen_int> ammo;
    std::vector<point_rel_ms> patrol_points_rel_ms;
};

#endif // CATA_SRC_MAPGEN_SPAWN_DATA_H
