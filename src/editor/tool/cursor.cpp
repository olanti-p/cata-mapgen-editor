#include "cursor.h"

#include <imgui/imgui.h>

namespace editor::tools
{
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
