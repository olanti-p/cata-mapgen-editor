#ifndef CATA_SRC_EDITOR_SELECTION_MASK_H
#define CATA_SRC_EDITOR_SELECTION_MASK_H

#include "common/bool.h"
#include "common/canvas_2d.h"
#include "cuboid_rectangle.h"
#include "point.h"

namespace editor
{

struct SelectionMask {
    public:
        SelectionMask() : data( point_zero ) {};
        explicit SelectionMask( point size ) : data( size ) {}
        explicit SelectionMask( Canvas2D<Bool> &&data ) : data( data ) {
            refresh_num_selected();
        }
        ~SelectionMask() = default;

        SelectionMask( const SelectionMask & ) = default;
        SelectionMask( SelectionMask && ) = default;
        SelectionMask &operator=( const SelectionMask & ) = default;
        SelectionMask &operator=( SelectionMask && ) = default;

        void clear_all();
        void set_all();
        inline void set( point pos ) {
            if( !data.get( pos ) ) {
                num_selected += 1;
            }
            data.set( pos, Bool( true ) );
        }
        inline void clear( point pos ) {
            if( data.get( pos ) ) {
                num_selected -= 1;
            }
            data.set( pos, Bool( false ) );
        }
        inline bool get( point pos ) const {
            return data.get( pos );
        }
        inline point get_size() const {
            return data.get_size();
        }
        inline half_open_rectangle<point> get_bounds() const {
            return data.get_bounds();
        }

        inline bool has_selected() const {
            return num_selected > 0;
        }

        const Canvas2D<Bool> &get_raw() const {
            return data;
        }

        void serialize( JsonOut &jsout ) const;
        void deserialize( const TextJsonValue &jsin );

    private:
        inline void refresh_num_selected() {
            num_selected = 0;
            for( const Bool &b : data.get_data() ) {
                if( b ) {
                    num_selected++;
                }
            }
        }

        int num_selected = 0;
        Canvas2D<Bool> data;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_SELECTION_MASK_H
