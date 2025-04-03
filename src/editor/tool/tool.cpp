#include "tool.h"

#include "all_enum_values.h"
#include "common/canvas_2d.h"
#include "imgui.h"
#include "mapgen/mapgen.h"
#include "state/control_state.h"
#include "state/state.h"
#include "state/tools_state.h"
#include "state/ui_state.h"
#include "tool/brush.h"
#include "tool/bucket.h"
#include "tool/cursor.h"
#include "tool/line_tool.h"
#include "tool/rectangle_tool.h"
#include "tool/selection.h"
#include "widget/widgets.h"

#include <unordered_map>

namespace editor::tools
{

bool ToolTarget::is_hovered_over_canvas() const
{
    return view_hovered && has_canvas && mapgen.get_bounds().contains( cursor_tile_pos );
}

const ToolDefinition &get_tool_definition( ToolKind kind )
{
    switch( kind ) {
        case ToolKind::Brush: {
            static Brush tool;
            return tool;
        }
        case ToolKind::Bucket: {
            static Bucket tool;
            return tool;
        }
        case ToolKind::Cursor: {
            static Cursor tool;
            return tool;
        }
        case ToolKind::Line: {
            static Line tool;
            return tool;
        }
        case ToolKind::Rectangle: {
            static Rectangle tool;
            return tool;
        }
        case ToolKind::Selection: {
            static Selection tool;
            return tool;
        }
        case ToolKind::_Num:
            break;
    }
    debugmsg( "Invalid ToolKind" );
    abort();
}

} // namespace editor::tools

namespace editor
{
void show_toolbar( State &state, bool &show )
{
    if( !ImGui::Begin( "Toolbar", &show,
                       ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize
                     ) ) {
        ImGui::End();
        return;
    }

    ToolsState &tools = *state.ui->tools;
    ControlState &control = *state.control;

    auto all_tools = all_enum_values<tools::ToolKind>();

    for( tools::ToolKind kind : all_tools ) {
        const tools::ToolDefinition &def = tools::get_tool_definition( kind );
        if( ImGui::RadioButton( def.get_tool_display_name().c_str(), tools.get_tool() == kind ) ) {
            if( !control.has_ongoing_tool_operation() ) {
                tools.set_tool( kind );
            }
        }
        ImGui::HelpPopup( def.get_tool_hint().c_str() );
    }

    ImGui::Separator();
    tools.get_settings( tools.get_tool() ).show();

    ImGui::End();
}
} // namespace editor
