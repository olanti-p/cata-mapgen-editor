#ifndef CATA_SRC_EDITOR_TOOLS_STATE_H
#define CATA_SRC_EDITOR_TOOLS_STATE_H

#include "mapgen_map_key.h"
#include "common/uuid.h"
#include "tool/tool.h"

#include <memory>
#include <unordered_map>

namespace editor
{
struct State;

struct ToolsState {
    public:
        ToolsState();
        ~ToolsState();

        ToolsState( const ToolsState & ) = delete;
        ToolsState( ToolsState && ) = default;

        ToolsState &operator=( const ToolsState & ) = delete;
        ToolsState &operator=( ToolsState && ) = default;

        void serialize( JsonOut &jsout ) const;
        void deserialize(const TextJsonObject&jsin );

        inline const map_key &get_main_tile() const {
            return selected_tile;
        }
        void set_main_tile( const map_key &uuid );

        inline tools::ToolKind get_tool() const {
            return tool;
        }
        void set_tool( tools::ToolKind t );

        tools::ToolSettings &get_settings( tools::ToolKind t );

    private:
        std::unordered_map<tools::ToolKind, std::unique_ptr<tools::ToolSettings>> tool_settings;
        tools::ToolKind tool = tools::ToolKind::Cursor;
        map_key selected_tile;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_TOOLS_STATE_H
