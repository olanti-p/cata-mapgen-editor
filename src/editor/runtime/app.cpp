#include "app.h"

#include "editor_engine.h"
#include <imgui/imgui.h>
#include "title_screen.h"

#include "state/control_state.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "project/project.h"

namespace editor
{
App::App() = default;
App::App( App && ) = default;
App::~App() = default;

App &App::operator=( App && ) = default;

void init_app( App &app )
{
    app.title_state = std::make_unique<TitleScreen>();
}

void show_app( App &app )
{
    if( app.editor_state ) {
        show_me_ui( *app.editor_state );
    } else if( app.title_state ) {
        show_title_screen( *app.title_state );
    }
}

static void clear_inputs()
{
    // Make sure our mouse click / etc. does not propagate into editing action
    ImGui::GetIO().ClearEventsQueue();
    ImGui::GetIO().ClearInputCharacters();
    ImGui::GetIO().ClearInputKeys();
}

void update_app_state( App &app )
{
    if( app.title_state && app.title_state->ret ) {
        const editor::TitleScreenReturn &retval = *app.title_state->ret;
        if( retval.exit ) {
            if( retval.exit_to_desktop ) {
                app.run_state.do_exit_to_dektop = true;
            } else {
                app.run_state.do_exit_to_game = true;
            }
        } else if( retval.make_new ) {
            clear_inputs();
            app.editor_state = std::make_unique<State>();
            std::string project_uuid = app.editor_state->project().project_uuid;
            set_project_ini_path( project_uuid );
        } else if( retval.load_existing ) {
            clear_inputs();
            std::unique_ptr<Project> f = std::make_unique<Project>();
            auto reader = [&]( JSON_IN & jsin ) {
                f->deserialize( jsin.get_value() );
            };
            if( read_from_file_text_json( cata_path(cata_path::root_path::unknown, retval.load_path), reader ) ) {
                app.editor_state = std::make_unique<State>( std::move( f ), &retval.load_path );
                std::string project_uuid = app.editor_state->project().project_uuid;
                set_project_ini_path( project_uuid );
                app.editor_state->control->reimport_all_palettes = true;
            } else {
                app.title_state->popup_prompt =
                    string_format( "Failed to load project:\n%s\nSee debug.log for details.", retval.load_path );
            }
        }
        app.title_state->ret.reset();
    } else if( app.editor_state && !app.editor_state->control->is_editor_running ) {
        app.editor_state.reset();
        set_default_ini_path();
    }
}

} // namespace editor
