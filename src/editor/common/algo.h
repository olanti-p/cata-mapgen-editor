#ifndef CATA_SRC_EDITOR_ALGO_H
#define CATA_SRC_EDITOR_ALGO_H

#include "uuid.h"
#include "canvas_2d.h"
#include "mapgen_map_key.h"

#include <vector>

namespace editor
{

/**
 * Find all tiles that match predicate.
*/
std::vector<point> find_tiles_via_global(
    const Canvas2D<map_key> &canvas,
    std::function<bool( point p, const map_key& )> predicate
);

/**
 * Find via floodfill all tiles that match predicate.
*/
std::vector<point> find_tiles_via_floodfill(
    const Canvas2D<map_key> &canvas,
    const point &initial_pos,
    std::function<bool( point p, const map_key& )> predicate
);

std::vector<point> line_bresenham( point a, point b );

/**
 * Given 2 opposite rectangle corners, returns lower-left and upper-right corners (min coords & max coords)
 */
template<typename Point>
std::pair<Point, Point> normalize_rect( Point a, Point b )
{
    return {
        Point( std::min( a.x(), b.x() ), std::min( a.y(), b.y() ) ),
        Point( std::max( a.x(), b.x() ), std::max( a.y(), b.y() ) )
    };
}

} // namespace editor

#endif // CATA_SRC_EDITOR_ALGO_H
