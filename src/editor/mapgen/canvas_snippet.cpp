#include "canvas_snippet.h"

namespace editor
{

CanvasSnippet make_snippet( const Canvas2D<UUID> &canvas, const SelectionMask &selection )
{
    assert( canvas.get_size() == selection.get_size() );
    assert( selection.has_selected() );

    int x_min = INT_MAX;
    int y_min = INT_MAX;
    int x_max = INT_MIN;
    int y_max = INT_MIN;

    for( int y = 0; y < selection.get_size().y; y++ ) {
        for( int x = 0; x < selection.get_size().x; x++ ) {
            if( selection.get( point( x, y ) ) ) {
                x_min = std::min( x_min, x );
                x_max = std::max( x_max, x );
                y_min = std::min( y_min, y );
                y_max = std::max( y_max, y );
            }
        }
    }

    point p_min( x_min, y_min );
    point p_max( x_max, y_max );
    Canvas2D<UUID> new_data( p_max - p_min + point( 1, 1 ), UUID_INVALID );
    Canvas2D<Bool> new_mask( p_max - p_min + point( 1, 1 ), Bool( false ) );

    for( int y = p_min.y; y <= p_max.y; y++ ) {
        for( int x = p_min.x; x <= p_max.x; x++ ) {
            point p_src( x, y );
            if( selection.get( p_src ) ) {
                point p_dest = p_src - p_min;
                new_data.set( p_dest, canvas.get( p_src ) );
                new_mask.set( p_dest, Bool( true ) );
            }
        }
    }

    return CanvasSnippet( std::move( new_data ), std::move( new_mask ), p_min );
}

} // namespace editor
