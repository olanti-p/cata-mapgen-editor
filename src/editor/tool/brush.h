#ifndef CATA_SRC_EDITOR_TOOL_BRUSH_H
#define CATA_SRC_EDITOR_TOOL_BRUSH_H

#include "common/canvas_2d.h"
#include "tool.h"

namespace editor::tools
{

struct BrushSettings : public ToolSettings {

};

struct BrushControl : public ToolControl {
    bool is_stroke_active = false;
    bool stroke_changed_data = false;

    void handle_tool_operation( ToolTarget &target ) override;
    inline bool operation_in_progress() const override {
        return is_stroke_active;
    }

    void start_stroke();
    void end_stroke( ToolTarget &target );
    void apply( Canvas2D<MapKey> &canvas, point pos, MapKey new_value );
};

struct Brush : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;
    ImGuiKey get_hotkey() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<BrushControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<BrushSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_BRUSH_H
