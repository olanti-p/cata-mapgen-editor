#include "rectangle_tool.h"

#include "common/algo.h"
#include "coordinates.h"
#include "imgui.h"
#include "line.h"
#include "mapgen/mapgen.h"
#include "point.h"
#include <vector>

namespace editor::tools
{

std::string Rectangle::get_tool_display_name() const
{
    return "Rectangle";
}

std::string Rectangle::get_tool_hint() const
{
    return "Drag LMB to draw a rectangle.\n\n"
           "Hold Shift to draw a square.\n"
           "Press Esc while dragging to cancel.";
}

void RectangleSettings::show()
{
    ImGui::Checkbox( "Filled", &filled );
}

void RectangleControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.has_canvas ) {
        start.reset();
        return;
    }
    RectangleSettings &settings = *dynamic_cast<RectangleSettings *>( target.settings );
    if( target.view_hovered ) {
        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
            // Stroke start
            start = target.cursor_tile_pos;
        }
        if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) && start ) {
            // Stroke end
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_rectangle_end( target );
            Canvas2D<MapKey> &canvas = target.mapgen.base.canvas;

            std::vector<point> rect = make_rectangle( p1, p2, settings.filled );
            apply( canvas, rect, target.main_tile );
            start.reset();
            target.made_changes = true; // TODO: fix false positives
        }
        if( ImGui::IsKeyPressed( ImGuiKey_Escape ) ) {
            // Abort
            start.reset();
        }
        if( start ) {
            target.want_tooltip = true;
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_rectangle_end( target );
            std::vector<point> rect = make_rectangle( p1, p2, settings.filled );
            for( const point &p : rect ) {
                target.highlight.tiles.emplace_back( p );
            }
        }
    }
    if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
        start.reset();
    }
}

std::vector<point> RectangleControl::make_rectangle( point_abs_etile p1, point_abs_etile p2,
        bool filled ) const
{
    auto corners = editor::normalize_rect( p1, p2 );
    std::vector<point> ret;
    if( filled ) {
        for( int y = corners.first.y(); y <= corners.second.y(); y++ ) {
            for( int x = corners.first.x(); x <= corners.second.x(); x++ ) {
                ret.emplace_back( x, y );
            }
        }
    } else {
        int x1 = corners.first.x();
        int x2 = corners.second.x();
        int y1 = corners.first.y();
        int y2 = corners.second.y();

        for( int x = x1; x <= x2; x++ ) {
            ret.emplace_back( x, y1 );
            if( y1 != y2 ) {
                ret.emplace_back( x, y2 );
            }
        }
        for( int y = y1 + 1; y <= y2 - 1; y++ ) {
            ret.emplace_back( x1, y );
            if( x1 != x2 ) {
                ret.emplace_back( x2, y );
            }
        }
    }
    return ret;
}

void RectangleControl::apply( Canvas2D<MapKey> &canvas, const std::vector<point> &rect,
                              MapKey new_value )
{
    for( const auto &p : rect ) {
        if( canvas.get_bounds().contains( p ) ) {
            canvas.set( p, new_value );
        }
    }
}

point_abs_etile RectangleControl::get_rectangle_end( ToolTarget &target ) const
{
    if( start && ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
        point delta = target.cursor_tile_pos.raw() - start->raw();
        point delta_abs = delta.abs();
        int dist = std::min( delta_abs.x, delta_abs.y );
        point vec;
        if( delta.x > 0 ) {
            vec.x = 1;
        } else if( delta.x < 0 ) {
            vec.x = -1;
        }
        if( delta.y > 0 ) {
            vec.y = 1;
        } else if( delta.y < 0 ) {
            vec.y = -1;
        }
        return *start + ( vec * dist );
    } else {
        return target.cursor_tile_pos;
    }
}

void RectangleControl::show_tooltip( ToolTarget &target )
{
    if( !start ) {
        ImGui::Text( "<ERROR:null rectangle start>" );
        return;
    }
    point delta = ( *start - get_rectangle_end( target ) ).raw().abs();
    ImGui::Text( "X %d", delta.x + 1 );
    ImGui::Text( "Y %d", delta.y + 1 );
}

} // namespace editor::tools
