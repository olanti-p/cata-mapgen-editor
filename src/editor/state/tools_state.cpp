#include "tools_state.h"

#include "tool/cursor.h"
#include "tool/tool.h"
#include "widget/widgets.h"
#include "imgui.h"
#include <memory>

namespace editor
{
ToolsState::ToolsState() = default;
ToolsState::~ToolsState() = default;

void ToolsState::set_main_tile( const map_key &uuid )
{
    selected_tile = uuid;
}

void ToolsState::set_tool( tools::ToolKind t )
{
    tool = t;
}

tools::ToolSettings &ToolsState::get_settings( tools::ToolKind t )
{
    auto it = tool_settings.find( t );
    if( it == tool_settings.end() ) {
        auto it_new = tool_settings.insert( { t, tools::get_tool_definition( t ).make_settings() } );
        return *it_new.first->second;
    } else {
        return *it->second;
    }
}

} // namespace editor
