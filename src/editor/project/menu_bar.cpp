#include "menu_bar.h"

#include <imgui/imgui.h>
#include "state/history_state.h"
#include "state/save_export_state.h"
#include "state/ui_state.h"
#include "state/state.h"
#include "state/control_state.h"

namespace editor
{
void show_main_menu_bar( State &state )
{
    ControlState &control = *state.control;

    if( ImGui::BeginMainMenuBar() ) {
        if( ImGui::BeginMenu( "File" ) ) {
            if( ImGui::MenuItem( "Save", "Ctrl+S" ) ) {
                control.want_save = true;
            }
            if( ImGui::MenuItem( "Save As...", "Ctrl+Shift+S" ) ) {
                control.want_save_as = true;
            }
            ImGui::Separator();
            if( ImGui::MenuItem( "Export", "Ctrl+E" ) ) {
                control.want_export = true;
            }
            if( ImGui::MenuItem( "Export As...", "Ctrl+Shift+E" ) ) {
                control.want_export_as = true;
            }
            ImGui::Separator();
            if( ImGui::MenuItem( "Exit", "Ctrl+Q" ) ) {
                control.want_close = true;
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Edit" ) ) {
            if( ImGui::MenuItem( "Undo", "Ctrl+Z", nullptr, state.history->can_undo() ) ) {
                state.history->queue_undo();
            }
            if( ImGui::MenuItem( "Redo", "Ctrl+Shift+Z", nullptr, state.history->can_redo() ) ) {
                state.history->queue_redo();
            }
            if (ImGui::MenuItem( "Paste from clipboard", "Ctrl+Shift+V" )) {
                state.control->handle_import_rows_from_clipboard();
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Window" ) ) {
            ImGui::MenuItem( "Project Overview", nullptr, &state.ui->show_project_overview );
            ImGui::MenuItem( "Mapgen Info", nullptr, &state.ui->show_mapgen_info );
            ImGui::MenuItem( "Active Palette", nullptr, &state.ui->show_mapgen_palette_simple );
            ImGui::MenuItem( "Active Palette Details", nullptr, &state.ui->show_mapgen_palette_verbose );
            ImGui::MenuItem( "Active Placements", nullptr, &state.ui->show_mapgen_objects );
            ImGui::MenuItem( "History", nullptr, &state.ui->show_history );
            ImGui::MenuItem( "Toolbar", nullptr, &state.ui->show_toolbar );
            ImGui::MenuItem( "Camera Controls", nullptr, &state.ui->show_camera_controls );
            ImGui::Separator();
            ImGui::MenuItem( "ImGui Demo", nullptr, &state.ui->show_demo_wnd );
            ImGui::MenuItem( "Debug/Metrics", nullptr, &state.ui->show_metrics_wnd );
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Preferences" ) ) {
            if( ImGui::MenuItem( "Autosave..." ) ) {
                state.ui->show_autosave_params = true;
            }
            ImGui::MenuItem( "Warn on import issues", nullptr, &state.ui->warn_on_import_issues);
            ImGui::MenuItem( "Show OMT grid", nullptr, &state.ui->show_omt_grid );
            ImGui::MenuItem( "Show symbols on canvas", nullptr, &state.ui->show_canvas_symbols );
            ImGui::MenuItem( "Show sprites on canvas", nullptr, &state.ui->show_canvas_sprites );
            ImGui::MenuItem( "Show placements on canvas", nullptr, &state.ui->show_canvas_objects );
            ImGui::MenuItem( "Show fill_ter sprites", nullptr, &state.ui->show_fill_ter_sprites);
            ImGui::SliderFloat("Sprite opacity", &state.ui->canvas_sprite_opacity, 0.0, 1.0);
            ImGui::EndMenu();
        }

        {
            std::string fps = string_format("   AVG %.3f ms/frame [%.1f FPS]", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImVec2 sz = ImGui::CalcTextSize(fps.c_str());
            float wnd_w = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX(wnd_w - (sz.x + ImGui::GetStyle().ItemSpacing.x));
            ImGui::Text("%s", fps.c_str());
        }

        ImGui::EndMainMenuBar();
    }

    if( state.control->has_ongoing_tool_operation() ) {
        return;
    }

    if( ImGui::IsKeyDown( ImGuiKey_LeftCtrl ) && ImGui::IsKeyPressed( ImGuiKey_S ) ) {
        if( ImGui::IsKeyDown( ImGuiKey_LeftShift ) ) {
            control.want_save_as = true;
        } else {
            control.want_save = true;
        }
    }

    if( ImGui::IsKeyDown( ImGuiKey_LeftCtrl ) && ImGui::IsKeyPressed( ImGuiKey_E ) ) {
        if( ImGui::IsKeyDown( ImGuiKey_LeftShift ) ) {
            control.want_export_as = true;
        } else {
            control.want_export = true;
        }
    }

    if( ImGui::IsKeyDown( ImGuiKey_LeftCtrl ) && ImGui::IsKeyPressed( ImGuiKey_Q ) ) {
        control.want_close = true;
    }

    if( ImGui::IsKeyDown( ImGuiKey_LeftCtrl ) && ImGui::IsKeyPressed( ImGuiKey_Z ) ) {
        if( ImGui::IsKeyDown( ImGuiKey_LeftShift ) ) {
            if( state.history->can_redo() ) {
                state.history->queue_redo();
            }
        } else {
            if( state.history->can_undo() ) {
                state.history->queue_undo();
            }
        }
    }

    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_V)) {
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
            state.control->handle_import_rows_from_clipboard();
        }
    }
}

} // namespace editor
