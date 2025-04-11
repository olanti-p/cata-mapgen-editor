#ifndef CATA_SRC_EDITOR_TOOL_LINE_TOOL_H
#define CATA_SRC_EDITOR_TOOL_LINE_TOOL_H

#include "common/canvas_2d.h"
#include "coordinates.h"
#include "tool.h"

namespace editor::tools
{

struct LineControl : public ToolControl {
    std::optional<point_abs_etile> start;

    void handle_tool_operation( ToolTarget &target ) override;
    inline bool operation_in_progress() const override {
        return start.has_value();
    }

    std::vector<point> make_line( point_abs_etile p1, point_abs_etile p2 ) const;
    void apply( Canvas2D<MapKey> &canvas, const std::vector<point> &line, MapKey new_value );
    point_abs_etile get_line_end( ToolTarget &target ) const;

    void show_tooltip( ToolTarget &target ) override;
};

struct LineSettings : public ToolSettings {

};

struct Line : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<LineControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<LineSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_LINE_TOOL_H
