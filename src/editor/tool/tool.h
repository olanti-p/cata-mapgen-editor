#ifndef CATA_SRC_EDITOR_TOOL_H
#define CATA_SRC_EDITOR_TOOL_H

#include "common/map_key.h"
#include "common/uuid.h"
#include "coordinates.h"
#include "enum_traits.h"
#include "imgui.h"

#include <cassert>
#include <memory>
#include <string>

namespace editor
{
struct Mapgen;
struct SelectionMask;
struct SnippetsState;
} // namespace editor

namespace editor::tools
{

enum class ToolKind {
    Cursor,
    Ruler,
    Pipette,
    Brush,
    Bucket,
    Line,
    Rectangle,
    Selection,
    _Num,
};

struct ToolTarget;

struct ToolSettings {
    ToolSettings() = default;
    virtual ~ToolSettings() = default;

    virtual void serialize( JsonOut &jsout ) const;
    virtual void deserialize( JSON_IN &jsin );

    virtual void show() {};
};

struct ToolControl {
    ToolControl() = default;
    virtual ~ToolControl() = default;

    virtual void handle_tool_operation( ToolTarget &/*target*/ ) {}
    virtual bool operation_in_progress() const {
        return false;
    }

    virtual void show_tooltip( ToolTarget & /*target*/ ) {};
    virtual bool operates_on_snippets( ToolTarget & /*target*/ ) {
        return false;
    }
};

struct ToolDefinition {
    ToolDefinition() = default;
    virtual ~ToolDefinition() = default;

    virtual std::string get_tool_display_name() const = 0;
    virtual std::string get_tool_hint() const = 0;
    virtual ImGuiKey get_hotkey() const = 0;
    virtual std::unique_ptr<ToolControl> make_control() const = 0;
    virtual std::unique_ptr<ToolSettings> make_settings() const = 0;
};

struct ToolHighlight {
    std::vector<point_abs_etile> tiles;
    std::vector<std::pair<point_abs_etile, point_abs_etile>> areas;
    std::optional<ImColor> color;

    inline bool active() const {
        return !tiles.empty() || !areas.empty();
    }
};

struct ToolTarget {
    bool view_hovered = false;
    bool has_canvas = false;
    bool made_changes = false;
    bool want_tooltip = false;
    point_abs_etile cursor_tile_pos;
    point_abs_epos cursor_view_pos;
    Mapgen &mapgen;
    ToolSettings *settings;
    MapKey main_tile;
    ToolHighlight &highlight;
    SelectionMask *selection = nullptr;
    SnippetsState &snippets;

    bool is_hovered_over_canvas() const;
};

const ToolDefinition &get_tool_definition( ToolKind kind );

} // namespace editor::tools

template<>
struct enum_traits<editor::tools::ToolKind> {
    static constexpr editor::tools::ToolKind last = editor::tools::ToolKind::_Num;
};

namespace editor
{
struct State;
void show_toolbar( State &state, bool &show );
void handle_toolbar_hotkeys(State& state);

} // namespace editor

#endif // CATA_SRC_EDITOR_TOOL_H
