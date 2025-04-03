#ifndef CATA_SRC_EDITOR_2D_CANVAS_H
#define CATA_SRC_EDITOR_2D_CANVAS_H

#include "cuboid_rectangle.h"
#include "point.h"

#include <algorithm>
#include <vector>

namespace editor
{

template<typename T>
class Canvas2D
{
    private:
        point size;
        std::vector<T> data;

    public:
        Canvas2D( point size, T fill_value = T() ) {
            set_size( size, fill_value );
        }
        Canvas2D( const Canvas2D<T> & ) = default;
        Canvas2D( Canvas2D<T> && ) = default;
        Canvas2D &operator=( const Canvas2D<T> & ) = default;
        Canvas2D &operator=( Canvas2D<T> && ) = default;

        inline point get_size() const {
            return size;
        }

        void set_size( point new_size, T fill_value = T() ) {
            if( size == new_size ) {
                return;
            }
            // TODO: graciously transfer entries from old size
            size = new_size;
            data.clear();
            data.resize( size.x * size.y, fill_value );
        }

        inline void set( point pos, T val ) {
            data[ pos.y * size.x + pos.x ] = std::move( val );
        }

        inline const T &get( point pos ) const {
            return data[ pos.y * size.x + pos.x ];
        }

        inline T &get( point pos ) {
            return data[ pos.y * size.x + pos.x ];
        }

        inline std::vector<T> &get_data() {
            return data;
        }

        inline const std::vector<T> &get_data() const {
            return data;
        }

        inline void set_all( T val ) {
            std::fill( data.begin(), data.end(), val );
        }

        inline half_open_rectangle<point> get_bounds() const {
            return {
                point_zero,
                size
            };
        }
};

} // namespace editor

#endif // CATA_SRC_EDITOR_2D_CANVAS_H
