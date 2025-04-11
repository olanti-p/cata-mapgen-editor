#ifndef CATA_SRC_EDITOR_TOOL_PIPETTE_H
#define CATA_SRC_EDITOR_TOOL_PIPETTE_H

#include "tool.h"

namespace editor::tools
{

struct PipetteControl : public ToolControl {
    void handle_tool_operation( ToolTarget &target ) override;
    bool operation_in_progress() const override;
};

struct PipetteSettings : public ToolSettings {

};

struct Pipette : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;
    ImGuiKey get_hotkey() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<PipetteControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<PipetteSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_PIPETTE_H
