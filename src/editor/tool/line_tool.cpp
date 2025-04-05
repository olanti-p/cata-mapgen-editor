#include "line_tool.h"

#include "common/algo.h"
#include "coordinates.h"
#include "imgui.h"
#include "line.h"
#include "mapgen/mapgen.h"
#include "point.h"
#include <vector>

namespace editor::tools
{

std::string Line::get_tool_display_name() const
{
    return "Line";
}

std::string Line::get_tool_hint() const
{
    return "Drag LMB to draw a line.\n\n"
           "Hold Shift to snap the line to 45 degrees.\n"
           "Press Esc while dragging to cancel.";
}

void LineControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.has_canvas ) {
        start.reset();
        return;
    }
    if( target.view_hovered ) {
        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
            // Stroke start
            start = target.cursor_tile_pos;
        }
        if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) && start ) {
            // Stroke end
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_line_end( target );
            Canvas2D<map_key> &canvas = target.mapgen.base.canvas;
            std::vector<point> line = make_line( p1, p2 );
            apply( canvas, line, target.main_tile );
            start.reset();
            target.made_changes = true; // TODO: fix false positives
        }
        if( ImGui::IsKeyPressed( ImGuiKey_Escape ) ) {
            // Abort
            start.reset();
        }
        if( start ) {
            if( *start != target.cursor_tile_pos ) {
                target.want_tooltip = true;
            }
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_line_end( target );
            std::vector<point> line = make_line( p1, p2 );
            for( const point &p : line ) {
                target.highlight.tiles.emplace_back( p );
            }
        }
    }
    if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
        start.reset();
    }
}

std::vector<point> LineControl::make_line( point_abs_etile p1, point_abs_etile p2 ) const
{
    return editor::line_bresenham( p1.raw(), p2.raw() );
}

void LineControl::apply( Canvas2D<map_key> &canvas, const std::vector<point> &line, map_key new_value )
{
    for( const auto &p : line ) {
        if( canvas.get_bounds().contains( p ) ) {
            canvas.set( p, new_value );
        }
    }
}

point_abs_etile LineControl::get_line_end( ToolTarget &target ) const
{
    if( start && ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
        direction dir = direction_from( start->raw(), target.cursor_tile_pos.raw() );
        point vec = displace_XY( dir );
        int dist = square_dist( start->raw(), target.cursor_tile_pos.raw() );
        return *start + ( vec * dist );
    } else {
        return target.cursor_tile_pos;
    }
}

void LineControl::show_tooltip( ToolTarget &target )
{
    if( !start ) {
        ImGui::Text( "<ERROR:null line start>" );
        return;
    }
    point delta = ( *start - get_line_end( target ) ).raw().abs();
    if( delta.x != 0 ) {
        ImGui::Text( "X %d", delta.x + 1 );
    }
    if( delta.y != 0 ) {
        ImGui::Text( "Y %d", delta.y + 1 );
    }
}

} // namespace editor::tools
