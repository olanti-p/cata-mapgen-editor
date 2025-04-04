#ifndef CATA_SRC_EDITOR_TOOL_RECTANGLE_TOOL_H
#define CATA_SRC_EDITOR_TOOL_RECTANGLE_TOOL_H

#include "common/canvas_2d.h"
#include "coordinates.h"
#include "tool.h"

namespace editor::tools
{

struct RectangleControl : public ToolControl {
    std::optional<point_abs_etile> start;

    void handle_tool_operation( ToolTarget &target ) override;
    inline bool operation_in_progress() const override {
        return start.has_value();
    }

    std::vector<point> make_rectangle( point_abs_etile p1, point_abs_etile p2, bool filled ) const;
    void apply( Canvas2D<UUID> &canvas, const std::vector<point> &rect, UUID new_value );
    point_abs_etile get_rectangle_end( ToolTarget &target ) const;

    void show_tooltip( ToolTarget &target ) override;
};

struct RectangleSettings : public ToolSettings {
    bool filled = false;

    void serialize( JsonOut &jsout ) const override;
    void deserialize( JSON_IN &jsin ) override;

    void show() override;
};

struct Rectangle : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<RectangleControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<RectangleSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_RECTANGLE_TOOL_H
