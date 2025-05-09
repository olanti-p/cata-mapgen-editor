#ifndef CATA_SRC_EDITOR_MAPOBJECT_H
#define CATA_SRC_EDITOR_MAPOBJECT_H

#include "common/int_range.h"
#include "common/uuid.h"
#include "cuboid_rectangle.h"

#include <imgui/imgui.h>

#include <memory>

namespace editor
{
struct Mapgen;
struct Piece;
struct State;

struct MapObject {
    MapObject();
    MapObject( const MapObject &rhs );
    MapObject( MapObject && );
    ~MapObject();

    MapObject &operator=( const MapObject &rhs );
    MapObject &operator=( MapObject && );

    const UUID &get_uuid() const;
    void set_uuid( const UUID &uuid );

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );

    inclusive_rectangle<point> get_bounding_box() const {
        return inclusive_rectangle<point>(point(x.min, y.min), point(x.max, y.max));
    }

    std::unique_ptr<Piece> piece;
    IntRange x;
    IntRange y;
    IntRange repeat;
    ImVec4 color;
    bool visible = true;
};

/**
 * =============== Windows ===============
 */
void show_mapobjects( State &state, Mapgen &f, bool is_active, bool &show );

} // namespace editor

#endif // CATA_SRC_EDITOR_MAPOBJECT_H
