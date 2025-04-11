#ifndef CATA_SRC_EDITOR_TOOL_CURSOR_H
#define CATA_SRC_EDITOR_TOOL_CURSOR_H

#include "tool.h"

namespace editor::tools
{

struct CursorControl : public ToolControl {
    bool operation_in_progress() const override;
};

struct CursorSettings : public ToolSettings {

};

struct Cursor : public ToolDefinition {
    std::string get_tool_display_name() const override;
    std::string get_tool_hint() const override;
    ImGuiKey get_hotkey() const override;

    std::unique_ptr<ToolControl> make_control() const override {
        return std::make_unique<CursorControl>();
    }
    std::unique_ptr<ToolSettings> make_settings() const override {
        return std::make_unique<CursorSettings>();
    }
};

} // namespace editor::tools

#endif // CATA_SRC_EDITOR_TOOL_CURSOR_H
