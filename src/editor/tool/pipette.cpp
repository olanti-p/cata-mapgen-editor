#include "pipette.h"

#include <imgui/imgui.h>
#include "mapgen/mapgen.h"
#include "common/canvas_2d.h"

namespace editor::tools
{
void PipetteControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.has_canvas ) {
        return;
    }
    if ( target.view_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        Canvas2D<MapKey> &canvas = target.mapgen.base.canvas;
        point pos = target.cursor_tile_pos.raw();
        target.main_tile = canvas.get(pos);
    }
}

bool PipetteControl::operation_in_progress() const {
    return ImGui::IsKeyDown( ImGuiKey_ModAlt );
}

std::string Pipette::get_tool_display_name() const
{
    return "Pipette";
}

std::string Pipette::get_tool_hint() const
{
    return "Pick a tile from grid to use in tools.";
}

ImGuiKey Pipette::get_hotkey() const
{
    return ImGuiKey_None;
}

} // namespace editor::tools
