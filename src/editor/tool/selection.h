#ifndef CATA_SRC_EDITOR_TOOL_SELECTION_H
#define CATA_SRC_EDITOR_TOOL_SELECTION_H

#include "coordinates.h"
#include "tool.h"

namespace editor
{
struct SelectionMask;
} // namespace editor

namespace editor::tools
{

struct SelectionControl : public ToolControl {
    std::optional<point_abs_etile> start;
    std::optional<point_abs_etile> drag_start;
    std::optional<point> initial_snippet_pos;
    bool is_dragging_selection = false;
    bool is_dragging_snippet = false;
    bool dismissing_selection = false;
    bool selection_aborted = false;

    void handle_tool_operation( ToolTarget &target ) override;
    inline bool operation_in_progress() const override {
        return start.has_value();
    }

    std::vector<point> make_rectangle( point_abs_etile p1, point_abs_etile p2 ) const;
    void apply( SelectionMask &selection, const std::vector<point> &rect );
    point_abs_etile get_rectangle_end( ToolTarget &target ) const;

    void show_tooltip( ToolTarget &target ) override;
    bool operates_on_snippets( ToolTarget & /*target*/ ) override {
        return true;
    }
};

struct SelectionSettings : public ToolSettings {
    void serialize( JsonOut &jsout ) const override;
    void deserialize( JsonIn &jsin ) override;

    void show() override;
};

struct Selection : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<SelectionControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<SelectionSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_SELECTION_H
