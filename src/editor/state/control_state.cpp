#include "control_state.h"

#include <imgui/imgui.h>
#include "tool/cursor.h"
#include "tool/selection.h"

#include <memory>

namespace editor
{
ControlState::ControlState() = default;
ControlState::ControlState( ControlState && ) = default;
ControlState::~ControlState() = default;
ControlState &ControlState::operator=( ControlState && ) = default;

bool ControlState::has_ongoing_tool_operation()
{
    if( !tool_control ) {
        return false;
    }
    return tool_control->operation_in_progress();
}

tools::ToolControl &ControlState::get_tool_control( tools::ToolKind t )
{
    set_tool_control( t );
    return *tool_control;
}

void ControlState::set_tool_control( tools::ToolKind t )
{
    if( t != tool_control_kind || !tool_control ) {
        tool_control_kind = t;
        tool_control = tools::get_tool_definition( t ).make_control();
    }
}

void ControlState::show_warning_popup( const std::string &data )
{
    want_show_warning_popup = true;
    warning_popup_data = data;
}

void ControlState::handle_warning_popup()
{
    if( want_show_warning_popup ) {
        want_show_warning_popup = false;
        ImGui::OpenPopup( "Warning" );
    }

    if( ImGui::BeginPopupModal( "Warning" ) ) {
        ImGui::Text( "%s", warning_popup_data.c_str() );
        if( ImGui::Button( "Dismiss" ) ) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ControlState::handle_import_rows_from_clipboard()
{
    if ( has_ongoing_tool_operation() ) {
        return;
    }
    tools::ToolControl& tool = get_tool_control(tools::ToolKind::Selection);
    tools::SelectionControl* selection = dynamic_cast<tools::SelectionControl*>(&tool);
    if (!selection) {
        // Shouldn't heppen
        std::abort();
    }
    selection->should_import_from_clipboard = true;
}

} // namespace editor
