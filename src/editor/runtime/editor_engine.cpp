#include "editor_engine.h"

#include "app.h"
#include "state/ui_state_store.h"
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <imgui/imgui.h>

#include "game.h"
#include "ui_manager.h"
#include "path_info.h"
#include "point.h"
#include "output.h"
#include "input.h"

#include <stdio.h>
#include <SDL2/SDL.h>
#include <thread>

#ifdef DebugLog
#  undef DebugLog
#endif

#include <imgui/imgui_internal.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;
static std::string ini_file_path;
static editor::App *current_app = nullptr;

namespace editor
{
void set_default_ini_path( bool flush )
{
    if( flush ) {
        flush_ini_to_disk();
    }

    ini_file_path = PATH_INFO::config_dir() + "imgui.ini";
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = ini_file_path.c_str();

    ImGui::ClearIniSettings();
    ImGui::LoadIniSettingsFromDisk( io.IniFilename );
}

void set_project_ini_path( const std::string &project_uuid, bool flush )
{
    if( flush ) {
        flush_ini_to_disk();
    }

    ini_file_path = PATH_INFO::config_dir() + "imgui/" + project_uuid + ".ini";
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = ini_file_path.c_str();

    ImGui::ClearIniSettings();
    ImGui::LoadIniSettingsFromDisk( io.IniFilename );
}

void flush_ini_to_disk()
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui::SaveIniSettingsToDisk( io.IniFilename );
}

bool init_ui( SDL_Window &window_ref, SDL_Renderer &renderer_ref )
{
    window = &window_ref;
    renderer = &renderer_ref;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer( window, renderer );
    ImGui_ImplSDLRenderer2_Init( renderer );

    // Specify ini file path
    set_default_ini_path( false );

    return true;
}

void shutdown_ui()
{
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    window = nullptr;
    renderer = nullptr;
}

void render_ui()
{
    if( !editor::ui_exists() ) {
        return;
    }
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    editor::show_app( *current_app );

    // Rendering
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData( ImGui::GetDrawData(), renderer );
}

bool process_event( SDL_Event &event )
{
    if( !editor::ui_exists() ) {
        return false;
    }

    ImGui_ImplSDL2_ProcessEvent( &event );
    return true;
}

bool ui_exists()
{
    return current_app != nullptr;
}

bool show_cata_ui()
{
    return !current_app;
}
} // namespace editor

point::point( ImVec2 v )
{
    x = v.x;
    y = v.y;
}

point::operator ImVec2()
{
    return ImVec2( x, y );
}

namespace editor
{
void bnmt_entry_point()
{
    static bool settings_export_initialized = false;
    if( !settings_export_initialized ) {
        settings_export_initialized = true;
        editor::initialize_settings_export();
    }

    {
        App app;
        current_app = &app;

        init_app( app );

        g->invalidate_main_ui_adaptor();
        ui_manager::redraw();

        bool do_exit_to_desktop = false;

        for( ;; ) {
            inp_mngr.get_input_event();
            std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
            refresh_display();
            update_app_state( app );
            if( app.run_state.do_exit_to_dektop ) {
                do_exit_to_desktop = true;
                break;
            }
            if( app.run_state.do_exit_to_game ) {
                break;
            }
        }
        current_app = nullptr;

        if( do_exit_to_desktop ) {
            std::exit( 0 );
        }
    }

    ui_manager::redraw();
    refresh_display();
}
} // namespace editor
