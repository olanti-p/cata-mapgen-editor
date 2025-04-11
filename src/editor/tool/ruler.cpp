#include "ruler.h"

#include <vector>

#include "common/algo.h"
#include "common/color.h"
#include "coordinates.h"
#include "imgui.h"
#include "line.h"
#include "mapgen/mapgen.h"
#include "point.h"

namespace editor::tools
{

std::string Ruler::get_tool_display_name() const
{
    return "Ruler";
}

std::string Ruler::get_tool_hint() const
{
    return "Drag LMB to measure distance and area.\n\n"
           "Hold Shift to snap to square or straight line.\n"
           "Press Esc while dragging to cancel.";
}


ImGuiKey Ruler::get_hotkey() const
{
    return ImGuiKey_Q;
}

void RulerControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.has_canvas ) {
        start.reset();
        return;
    }
    RulerSettings &settings = *dynamic_cast<RulerSettings *>( target.settings );
    if( target.view_hovered ) {
        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
            start = target.cursor_tile_pos;
        }
        if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) && start ) {
            start.reset();
        }
        if( ImGui::IsKeyPressed( ImGuiKey_Escape ) ) {
            start.reset();
        }
        if( start ) {
            target.want_tooltip = true;
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_selection_end( target );
            target.highlight.areas.emplace_back( p1, p2 );
            target.highlight.color = col_ruler;
        }
    }
    if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
        start.reset();
    }
}

point_abs_etile RulerControl::get_selection_end( ToolTarget &target ) const
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

void RulerControl::show_tooltip( ToolTarget &target )
{
    if( !start ) {
        ImGui::Text( "<ERROR:null ruler start>" );
        return;
    }
    point delta = ( *start - get_selection_end( target ) ).raw().abs();
    if (delta.x != 0) {
        ImGui::Text( "X %d", delta.x + 1 );
    }
    if (delta.y != 0) {
        ImGui::Text( "Y %d", delta.y + 1 );
    }
    if (delta.x != 0 && delta.y != 0) {
        ImGui::Text( "AREA %d", (delta.x + 1) * (delta.y + 1) );
    }
}

} // namespace editor::tools
