#include "title_screen.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "widget/widgets.h"
#include "widget/ImGuiFileDialog.h"

#include "point.h"
#include "game.h"

namespace editor
{

static void show_titlescreen_window( TitleScreen &state )
{
    ImGui::Begin( "##title-screen-wnd", nullptr,
                  ImGuiWindowFlags_AlwaysAutoResize |
                  ImGuiWindowFlags_NoDecoration |
                  ImGuiWindowFlags_NoMove
                );
    ImGui::TextCentered( "Welcome to" );
    ImGui::TextCentered( "Mapgen Editor" );
    ImGui::Text(" ");
    ImGui::Separator();

    // Controls
    ImVec2 btn_size( ImGui::GetFrameHeight() * 8.0f, ImGui::GetFrameHeight() * 2.0f );
    if( ImGui::Button( "New Project", btn_size ) ) {
        state.ret = TitleScreenReturn();
        state.ret->make_new = true;
    }
    if( ImGui::Button( "Load Project", btn_size ) ) {
        state.open_project_dialog = true;
    }
    if( ImGui::Button( "Exit Editor", btn_size ) ) {
        state.ret = TitleScreenReturn();
        state.ret->exit = true;
    }
    if( ImGui::Button( "Exit To Desktop", btn_size ) ) {
        state.ret = TitleScreenReturn();
        state.ret->exit = true;
        state.ret->exit_to_desktop = true;
    }

    if( state.open_project_dialog ) {
        state.open_project_dialog = false;
        ImGui::SetNextWindowSize( ImVec2( 580, 380 ), ImGuiCond_FirstUseEver );
        ImGuiFileDialog::Instance()->OpenDialog( "OpenFile", "Choose Project File", ".json", "." );
    }

    if( ImGuiFileDialog::Instance()->Display( "OpenFile" ) ) {
        if( ImGuiFileDialog::Instance()->IsOk() ) {
            std::map<std::string, std::string> selections = ImGuiFileDialog::Instance()->GetSelection();
            if( selections.size() == 1 ) {
                state.ret = TitleScreenReturn();
                state.ret->load_existing = true;
                for( const auto &sel : selections ) {
                    state.ret->load_path = sel.second;
                    break;
                }
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if( g->load_editor_project_on_start ) {
        state.ret = TitleScreenReturn();
        state.ret->load_existing = true;
        state.ret->load_path = *g->load_editor_project_on_start;
        g->load_editor_project_on_start.reset();
    }

    if( state.popup_prompt && !ImGui::IsPopupOpen( "Error" ) ) {
        ImGui::OpenPopup( "Error" );
    }

    if( ImGui::BeginPopupModal( "Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) ) {
        ImGui::Text( "%s", state.popup_prompt->c_str() );
        if( ImGui::Button( "Ok" ) ) {
            ImGui::CloseCurrentPopup();
            state.popup_prompt.reset();
        }
        ImGui::EndPopup();
    }

    ImGui::SetWindowPos(
        ImGui::GetIO().DisplaySize / 2.0f -
        ImGui::GetWindowSize() / 2.0f
    );

    ImGui::End();
}

void show_title_screen( TitleScreen &state )
{
    show_titlescreen_window( state );
}

} // namespace editor
