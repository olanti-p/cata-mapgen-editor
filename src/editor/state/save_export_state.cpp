#include "save_export_state.h"

#include "game.h"

#include "common/timestamp.h"
#include "history_state.h"
#include "imgui.h"
#include "path_info.h"
#include "project/project.h"
#include "project/project_export.h"
#include "state.h"
#include "state/control_state.h"
#include "tools_state.h"
#include "ui_state.h"
#include "widget/ImGuiFileDialog.h"
#include "widget/widgets.h"

#include <optional>
#include <vector>
#include <filesystem>

namespace editor
{
static std::string autosave_folder()
{
    return PATH_INFO::config_dir() + "autosave/";
}

static void prune_old_autosaves( int num_autosaves_to_keep )
{
    std::vector<std::string> files;
    std::string ext( ".json" );
    for( auto &p : std::filesystem::directory_iterator( autosave_folder() ) ) {
        if( p.path().extension() == ext ) {
            files.push_back( p.path().stem().string() );
        }
    }

    std::sort( files.begin(), files.end() );

    for( int i = 0; i < num_autosaves_to_keep; i++ ) {
        if( files.empty() ) {
            break;
        }
        files.pop_back();
    }

    for( const std::string &file : files ) {
        std::string path = autosave_folder() + file + ext;
        std::filesystem::remove( path );
    }
}

void handle_project_autosave( State &state )
{
    if( !state.ui->autosave_enabled ) {
        return;
    }
    SaveExportState &sestate = *state.save_export;
    sestate.elapsed_since_autosave += ImGui::GetIO().DeltaTime;
    if( sestate.elapsed_since_autosave <= state.ui->autosave_interval ) {
        return;
    }
    if( state.control->has_ongoing_tool_operation() ) {
        return;
    }
    std::optional<SnapshotNumber> &autosaved_snapshot = state.history->last_autosaved_snapshot;
    SnapshotNumber current_snapshot = state.history->current_snapshot.num;
    if( autosaved_snapshot && *autosaved_snapshot == current_snapshot ) {
        return;
    }
    std::string autosave_path = autosave_folder() + get_timestamp_ms_now() + ".json";
    write_to_file( autosave_path, [&]( std::ostream & oss ) {
        oss << serialize( state.project() );
    } );
    sestate.elapsed_since_autosave = 0.0f;
    autosaved_snapshot = current_snapshot;
    prune_old_autosaves( state.ui->autosave_limit );
}

void handle_project_saving( State &state )
{
    ControlState &control = *state.control;
    SaveExportState &sestate = *state.save_export;

    if( state.control->has_ongoing_tool_operation() ) {
        control.want_save = false;
        control.want_save_as = false;
        control.want_exit_after_save = false;
        return;
    }

    if( control.want_save && !sestate.project_save_path ) {
        control.want_save = false;
        control.want_save_as = true;
    }

    if( control.want_save_as ) {
        control.want_save_as = false;
        ImGui::SetNextWindowSize( ImVec2( 580, 380 ), ImGuiCond_FirstUseEver );
        ImGuiFileDialog::Instance()->OpenDialog( "SaveToFile",
                "Save As...", ".json",
                sestate.project_save_path ? *sestate.project_save_path : ".",
                1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite );
    }

    if( ImGuiFileDialog::Instance()->Display( "SaveToFile" ) ) {
        if( ImGuiFileDialog::Instance()->IsOk() ) {
            sestate.project_save_path = ImGuiFileDialog::Instance()->GetFilePathName();
            control.want_save = true;
        } else {
            control.want_exit_after_save = false;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if( control.want_save ) {
        control.want_save = false;
        assert( sestate.project_save_path );
        write_to_file( *sestate.project_save_path, [&]( std::ostream & oss ) {
            oss << serialize( state.project() );
        } );
        state.history->last_saved_snapshot = state.history->current_snapshot.num;
        if( control.want_exit_after_save ) {
            control.want_exit_after_save = false;
            control.is_editor_running = false;
        }
    }
}

void handle_project_exporting( State &state )
{
    ControlState &control = *state.control;
    UiState &ui = *state.ui;

    if( state.control->has_ongoing_tool_operation() ) {
        control.want_export = false;
        control.want_export_as = false;
        return;
    }

    if( control.want_export && (!ui.project_export_path || ui.project_export_path->empty() ) ) {
        control.want_export = false;
        control.want_export_as = true;
    }

    if( control.want_export_as ) {
        control.want_export_as = false;
        ImGui::SetNextWindowSize( ImVec2( 580, 380 ), ImGuiCond_FirstUseEver );
        ImGuiFileDialog::Instance()->OpenDialog( "ExportToFile",
                "Export As...", ".json",
                ui.project_export_path ? *ui.project_export_path : ".",
                1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite );
    }

    if( ImGuiFileDialog::Instance()->Display( "ExportToFile" ) ) {
        if( ImGuiFileDialog::Instance()->IsOk() ) {
            ui.project_export_path = ImGuiFileDialog::Instance()->GetFilePathName();
            control.want_export = true;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if( g->export_editor_project_on_start ) {
        ui.project_export_path = *g->export_editor_project_on_start;
        control.want_export = true;
        g->export_editor_project_on_start.reset();
    }

    if( control.want_export ) {
        control.want_export = false;
        assert( ui.project_export_path );
        write_to_file( *ui.project_export_path, [&]( std::ostream & oss ) {
            std::string s = editor_export::to_string( state.project() );
            oss << editor_export::format_string( s );
        } );
        state.history->last_exported_snapshot = state.history->current_snapshot.num;
    }
}

void handle_project_exiting( State &state )
{
    ControlState &control = *state.control;

    if( state.control->has_ongoing_tool_operation() ) {
        control.want_close = false;
        return;
    }

    if( control.want_close ) {
        if( state.history->has_unsaved_changes() ) {
            ImGui::OpenPopup( "###warn-unsaved-on-close" );
            control.want_close = false;
        } else {
            control.is_editor_running = false;
        }
    }

    if( ImGui::BeginPopupModal( "###warn-unsaved-on-close", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::TextCentered( "Do you want to save the changes?" );
        ImGui::Text( " " );
        ImVec2 btn_sz( ImGui::GetFrameHeight() * 5.0f, ImGui::GetFrameHeight() );
        if( ImGui::Button( "Don't Save", btn_sz ) ) {
            ImGui::CloseCurrentPopup();
            control.is_editor_running = false;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Cancel", btn_sz ) ) {
            control.want_close = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if( ImGui::Button( "Save", btn_sz ) ) {
            ImGui::CloseCurrentPopup();
            control.want_exit_after_save = true;
            control.want_save = true;
        }
        ImGui::EndPopup();
    }
}

void show_autosave_settings( UiState &ui, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 230.0f, 130.0f ), ImGuiCond_FirstUseEver );
    if( !ImGui::Begin( "Autosave Settings", &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::Checkbox( "Enabled", &ui.autosave_enabled );
    ImGui::BeginDisabled( !ui.autosave_enabled );
    ImGui::HelpMarkerInline(
        "Amount of autosaves that will be kept on disk.\n\n"
        "Oldest autosaves are removed first.\n\n"
        "WARNING: This is a per-project setting, but autosave slots are shared between projects!\n\n"
        "TODO: make this a persistent global option saved in \"options.json\"."
    );
    ImGui::InputIntClamped( "Limit", ui.autosave_limit, 1, 100,
                            ImGuiInputTextFlags_AutoSelectAll );
    ImGui::HelpMarkerInline(
        "Interval in seconds between autosaves.\n\n"
        "Autosave won't be created if there were no changes in the project."
    );
    ImGui::InputIntClamped( "Interval", ui.autosave_interval, 5, 3600,
                            ImGuiInputTextFlags_AutoSelectAll );
    ImGui::EndDisabled();

    ImGui::End();
}

} // namespace editor
