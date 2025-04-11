#ifndef CATA_SRC_EDITOR_TOOL_RULER_H
#define CATA_SRC_EDITOR_TOOL_RULER_H

#include "coordinates.h"
#include "tool.h"

namespace editor::tools
{

struct RulerControl : public ToolControl {
    std::optional<point_abs_etile> start;

    void handle_tool_operation( ToolTarget &target ) override;
    inline bool operation_in_progress() const override {
        return start.has_value();
    }

    point_abs_etile get_selection_end( ToolTarget &target ) const;

    void show_tooltip( ToolTarget &target ) override;
};

struct RulerSettings : public ToolSettings {

};

struct Ruler : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;
    ImGuiKey get_hotkey() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<RulerControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<RulerSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_RULER_H
