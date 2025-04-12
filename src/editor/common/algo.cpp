#include "algo.h"

#include "point.h"
#include <imgui/imgui.h>

#include <set>

namespace editor
{

std::vector<point> find_tiles_via_global( const Canvas2D<MapKey> &canvas,
        std::function<bool( point p, const MapKey& )> predicate )
{
    std::vector<point> ret;

    for( int x = 0; x < canvas.get_size().x; x++ ) {
        for( int y = 0; y < canvas.get_size().y; y++ ) {
            point p( x, y );
            const MapKey&t = canvas.get( p );
            if( predicate( p, t ) ) {
                ret.push_back( p );
            }
        }
    }

    return ret;
}

std::vector<point> find_tiles_via_floodfill( const Canvas2D<MapKey> &canvas,
        const point &initial_pos,
        std::function<bool( point p, const MapKey& )> predicate )
{
    std::vector<point> ret;

    if( !predicate( initial_pos, canvas.get( initial_pos ) ) ) {
        return ret;
    }

    std::set<point> open;
    std::set<point> closed;
    open.insert( initial_pos );

    while( !open.empty() ) {
        auto it = open.cbegin();
        point p = *it;
        open.erase( it );
        closed.insert( p );
        ret.push_back( p );
        for( const point &d : neighborhood ) {
            point p2 = p + d;
            if( p2.x < 0 || p2.y < 0 || p2.x >= canvas.get_size().x || p2.y >= canvas.get_size().y ) {
                continue;
            }
            if( closed.count( p2 ) != 0 ) {
                continue;
            }
            closed.insert( p2 );
            if( predicate( p2, canvas.get( p2 ) ) ) {
                open.insert( p2 );
            }
        }
    }

    return ret;
}

std::vector<point> line_bresenham( point a, point b )
{
    std::vector<point> ret;
    int x0 = a.x;
    int y0 = a.y;
    int x1 = b.x;
    int y1 = b.y;
    int dx = std::abs( x1 - x0 );
    int sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs( y1 - y0 );
    int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while( true ) {
        ret.emplace_back( x0, y0 );
        if( x0 == x1 && y0 == y1 ) {
            break;
        }
        int e2 = 2 * error;
        if( e2 >= dy ) {
            if( x0 == x1 ) {
                break;
            }
            error = error + dy;
            x0 = x0 + sx;
        }
        if( e2 <= dx ) {
            if( y0 == y1 ) {
                break;
            }
            error = error + dx;
            y0 = y0 + sy;
        }
    }

    return ret;
}

ImVec2 vector_rotated(ImVec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return ImVec2(
        v.x * c - v.y * s,
        v.x * s + v.y * c
    );
}

} // namespace editor
