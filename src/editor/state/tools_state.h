#ifndef CATA_SRC_EDITOR_TOOLS_STATE_H
#define CATA_SRC_EDITOR_TOOLS_STATE_H

#include "common/map_key.h"
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

        inline const MapKey &get_main_tile() const {
            return selected_tile;
        }
        void set_main_tile( const MapKey &uuid );

        inline tools::ToolKind get_tool() const {
            if (pipette_active) {
                return tools::ToolKind::Pipette;
            }
            else {
                return tool;
            }
        }
        void set_tool( tools::ToolKind t );
        void set_is_pipette_override( bool pipette_active_ ) {
            pipette_active = pipette_active_;
        }

        tools::ToolSettings &get_settings( tools::ToolKind t );

    private:
        std::unordered_map<tools::ToolKind, std::unique_ptr<tools::ToolSettings>> tool_settings;
        tools::ToolKind tool = tools::ToolKind::Cursor;
        MapKey selected_tile;
        bool pipette_active;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_TOOLS_STATE_H
