#ifndef CATA_SRC_EDITOR_CANVAS_SNIPPET_H
#define CATA_SRC_EDITOR_CANVAS_SNIPPET_H

#include "common/bool.h"
#include "common/canvas_2d.h"
#include "common/uuid.h"
#include "mapgen/selection_mask.h"
#include "point.h"
#include <optional>

namespace editor
{

struct CanvasSnippet {
    public:
        CanvasSnippet()
            : data( point_zero ), mask( point_zero ), pos( point_zero ) {}
        CanvasSnippet( Canvas2D<UUID> &&data, Canvas2D<Bool> &&mask, point pos )
            : data( data ), mask( SelectionMask( std::move( mask ) ) ), pos( pos ) {}
        ~CanvasSnippet() = default;

        CanvasSnippet( const CanvasSnippet & ) = default;
        CanvasSnippet( CanvasSnippet && ) = default;
        CanvasSnippet &operator=( const CanvasSnippet & ) = default;
        CanvasSnippet &operator=( CanvasSnippet && ) = default;

        inline point get_size() const {
            return data.get_size();
        }
        inline point get_pos() const {
            return pos;
        }
        inline void set_pos( point new_pos ) {
            pos = new_pos;
        }

        inline std::optional<UUID> get_data_at( point pos ) const {
            if( mask.get( pos ) ) {
                return data.get( pos );
            } else {
                return std::nullopt;
            }
        }

        inline half_open_rectangle<point> get_bounds() const {
            return data.get_bounds();
        }

        inline const SelectionMask &get_selection_mask() const {
            return mask;
        }

        void serialize( JsonOut &jsout ) const;
        void deserialize( const TextJsonValue &jsin );

    private:
        Canvas2D<UUID> data;
        SelectionMask mask;
        point pos;
};

CanvasSnippet make_snippet( const Canvas2D<UUID> &canvas, const SelectionMask &selection );

} // namespace editor

#endif // CATA_SRC_EDITOR_CANVAS_SNIPPET_H
