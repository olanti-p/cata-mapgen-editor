#include "cursor.h"

#include <imgui/imgui.h>

namespace editor::tools
{
bool CursorControl::operation_in_progress() const {
    return ImGui::IsMouseDown( ImGuiMouseButton_Left );
}

std::string Cursor::get_tool_display_name() const
{
    return "Cursor";
}

std::string Cursor::get_tool_hint() const
{
    return "Does nothing.  The true neutral choice!\n\n"
        "Drag LMB to pan view.";
}

ImGuiKey Cursor::get_hotkey() const
{
    return ImGuiKey_C;
}

} // namespace editor::tools
