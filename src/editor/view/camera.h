#ifndef CATA_SRC_EDITOR_CAMERA_H
#define CATA_SRC_EDITOR_CAMERA_H

#include "coordinates.h"

namespace editor
{

constexpr int MIN_SCALE = 8;
constexpr int MAX_SCALE = 128;
constexpr int DEFAULT_SCALE = 32;

struct Camera {
    void serialize( JsonOut &jsout ) const;
    void deserialize( JsonIn &jsin );

    point_abs_epos pos;
    point_rel_epos drag_delta;
    int scale = DEFAULT_SCALE;

    point_abs_epos screen_to_world( const point_abs_screen &p ) const;
    point_abs_screen world_to_screen( const point_abs_epos &p ) const;
    point_rel_epos screen_to_world( const point_rel_screen &p ) const;
    point_rel_screen world_to_screen( const point_rel_epos &p ) const;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_CAMERA_H
