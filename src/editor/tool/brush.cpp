#include "brush.h"

#include "common/canvas_2d.h"
#include <imgui/imgui.h>
#include "mapgen/mapgen.h"

#include <cassert>

namespace editor::tools
{

std::string Brush::get_tool_display_name() const
{
    return "Brush";
}

std::string Brush::get_tool_hint() const
{
    return "Hold LMB to draw with selected tile.";
}

ImGuiKey Brush::get_hotkey() const
{
    return ImGuiKey_W;
}

void BrushControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.is_hovered_over_canvas() ) {
        if( is_stroke_active ) {
            end_stroke( target );
        }
        return;
    }
    if( ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
        if( !is_stroke_active ) {
            start_stroke();
        }
        Canvas2D<MapKey> &canvas = target.mapgen.base.canvas;
        apply( canvas, target.cursor_tile_pos.raw(), target.main_tile );
    } else if( is_stroke_active ) {
        end_stroke( target );
    }
}

void BrushControl::start_stroke()
{
    assert( !is_stroke_active );
    is_stroke_active = true;
    stroke_changed_data = false;
}

void BrushControl::end_stroke( ToolTarget &target )
{
    assert( is_stroke_active );
    is_stroke_active = false;
    target.made_changes = stroke_changed_data;
}

void BrushControl::apply( Canvas2D<MapKey> &canvas, point pos, MapKey new_value )
{
    canvas.set( pos, new_value );
    stroke_changed_data = true;
}

} // namespace editor::tools
