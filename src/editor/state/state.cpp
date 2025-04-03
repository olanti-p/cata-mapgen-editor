#include "state.h"

#include "control_state.h"
#include "history_state.h"
#include "save_export_state.h"
#include "ui_state.h"
#include "project/project.h"

namespace editor
{

void show_me_ui( State &state )
{
    run_ui_for_state( state );
}

State::State() : State( create_empty_project() ) { }

State::State( std::unique_ptr<Project> &&project ) : State( std::move( project ),
            nullptr ) { }

State::State( std::unique_ptr<Project> &&project,
              const std::string *loaded_from_path ) : history( std::move( project ), !!loaded_from_path )
{
    if( loaded_from_path ) {
        save_export->project_save_path = *loaded_from_path;
    }
}

State::~State() = default;

State::State( State && ) = default;
State &State::operator=( State && ) = default;

Project &State::project()
{
    return history->project();
}

void State::mark_changed( const char *id )
{
    history->mark_changed( id );
}

bool State::is_changed() const
{
    return history->is_changed();
}

} // namespace editor
