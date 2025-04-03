#include "cursor.h"

#include "imgui.h"

namespace editor::tools
{

std::string Cursor::get_tool_display_name() const
{
    return "Cursor";
}

std::string Cursor::get_tool_hint() const
{
    return "Does nothing.  The true neutral choice!";
}

} // namespace editor::tools
