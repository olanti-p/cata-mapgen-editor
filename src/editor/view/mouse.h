#ifndef CATA_SRC_EDITOR_MOUSE_H
#define CATA_SRC_EDITOR_MOUSE_H

#include "coordinates.h"

namespace editor
{
struct Camera;

point_abs_screen get_mouse_pos();
point_abs_epos get_mouse_view_pos( const Camera &cam );
point_abs_etile get_mouse_tile_pos( const Camera &cam );

} // namespace editor

#endif // CATA_SRC_EDITOR_MOUSE_H
