#include "history_state.h"

#include "state/tools_state.h"
#include "project/project.h"
#include "state/ui_state.h"
#include "widget/widgets.h"

// Cata's DebugLog define conflicts with function in ImGui
#ifdef DebugLog
#  undef DebugLog
#endif

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace editor
{
ProjectSnapshot::ProjectSnapshot()
{
    project = std::make_unique<Project>();
}
ProjectSnapshot::ProjectSnapshot( ProjectSnapshot && ) = default;
ProjectSnapshot::~ProjectSnapshot() = default;
ProjectSnapshot &ProjectSnapshot::operator=( ProjectSnapshot && ) = default;

ProjectSnapshot ProjectSnapshot::make_copy() const
{
    ProjectSnapshot ret;
    ret.project = std::make_unique<Project>( *project );
    ret.num = num;
    return ret;
}

void show_edit_history( HistoryState &state, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 230.0f, 130.0f ), ImGuiCond_FirstUseEver );
    if( !ImGui::Begin( "Edit history", &show ) ) {
        ImGui::End();
        return;
    }

    ImGui::HelpMarkerInline(
        "Undo/redo support.\n\n"
        "To provide undo and redo functionality, the editor keeps track of snapshots (old and future versions) of the project.  "
        "This is done entirely in memory, so remembering too much snapshots may exhaust available RAM "
        "and slow down your OS or result in a crash.  You can manually control how much snapshots will be kept alive "
        "using the \"History limit\" widget.\n\n"
        "Hotkeys:\n"
        "  Ctrl+Z - Undo (advance to older snapshot)\n"
        "  Ctrl+Shift+Z - Redo (advance to newer snapshot)\n"
    );

    ImGui::SetNextItemWidth( ImGui::GetFrameHeight() * 4.0f );
    ImGui::InputIntClamped( "History limit", state.history_capacity, 10, 10000,
                            ImGuiInputTextFlags_AutoSelectAll );

    ImGui::HelpMarkerInline(
        "The list below keeps track of project snapshots.\n\n"
        "Click on a snapshot to switch to it.\n\n"
        "Markers used in the list:\n"
        "  [S] This snapshot is the one saved in the project file.\n"
        "  [E] This snapshot is the one that was used for export.\n"
        "  [A] This snapshot is the one that was autosaved.\n"
    );
    ImGui::Text( "Edit counter (debug): %d", state.edit_counter );

    for( const ProjectSnapshot &entry : state.snapshots ) {
        bool is_saved = state.last_saved_snapshot && *state.last_saved_snapshot == entry.num;
        bool is_exported = state.last_exported_snapshot && *state.last_exported_snapshot == entry.num;
        bool is_autosaved = state.last_autosaved_snapshot && *state.last_autosaved_snapshot == entry.num;
        std::string fname = string_format(
                                "Version %d%s%s%s",
                                entry.num,
                                is_saved ? " [S]" : "",
                                is_exported ? " [E]" : "",
                                is_autosaved ? " [A]" : ""
                            );
        if( ImGui::Selectable( fname.c_str(), entry.num == state.current_snapshot.num ) ) {
            state.switch_to_snapshot = entry.num;
        }
    }

    ImGui::End();
}

void handle_snapshot_change( HistoryState &state )
{
    if( state.switch_to_snapshot ) {
        auto it = std::find_if( state.snapshots.cbegin(),
        state.snapshots.cend(), [&]( const ProjectSnapshot & rev ) {
            return rev.num == *state.switch_to_snapshot;
        } );
        assert( it != state.snapshots.cend() );
        state.current_snapshot = it->make_copy();
        state.switch_to_snapshot.reset();
    } else if( state.project_has_changes ) {
        state.project_has_changes = false;

        const bool is_changing_same = state.last_widget_changed && state.current_widget_changed &&
                                      *state.last_widget_changed == *state.current_widget_changed;

        state.current_widget_changed_str.clear();
        state.last_widget_changed = state.current_widget_changed;
        state.current_widget_changed = std::nullopt;

        bool is_alt_history = false;

        // Erase alternative history
        while( state.snapshots[0].num != state.current_snapshot.num ) {
            // TODO: optimize this to use dequeue
            state.snapshots.erase( state.snapshots.cbegin() );
            is_alt_history = true;
        }

        const bool is_rev_saved = state.last_saved_snapshot ? *state.last_saved_snapshot ==
                                  state.current_snapshot.num : false;
        const bool is_rev_exported = state.last_exported_snapshot ? *state.last_exported_snapshot ==
                                     state.current_snapshot.num : false;
        const bool collapse_change = is_changing_same && !is_alt_history && !is_rev_saved &&
                                     !is_rev_exported && !state.snapshots.empty();

        if( collapse_change ) {
            state.snapshots.erase( state.snapshots.cbegin() );
        } else {
            state.current_snapshot.num++;
        }
        state.snapshots.insert( state.snapshots.cbegin(), state.current_snapshot.make_copy() );

        // Erase old entries
        if( static_cast<int>( state.snapshots.size() ) > state.history_capacity ) {
            state.snapshots.resize( state.history_capacity );
        }
    }
}

HistoryState::HistoryState( std::unique_ptr<Project> &&project, bool was_loaded )
{
    current_snapshot = ProjectSnapshot();

    if( was_loaded ) {
        last_saved_snapshot = current_snapshot.num;
    }

    if( project ) {
        current_snapshot.project = std::move( project );
    }

    snapshots.reserve( history_capacity + 1 );
    snapshots.emplace_back( current_snapshot.make_copy() );
}

void HistoryState::mark_changed( const char *id )
{
    std::string new_widget_changed_str = id ? id : "<nullptr>";
    if( project_has_changes ) {
        // FIXME: this gets truggered when pressing Ctrl+Z while editing text.
        // Very common occurance, must fix
        std::cerr << string_format(
                      R"(Tried to invoke mark_changed( "%s" ), but the project has already been marked as changed with id "%s".)",
                      new_widget_changed_str,
                      current_widget_changed_str
                  ) << std::endl;
        //std::abort();
        // Shouldn't happen, but we proceed anyway as a temporary workaround
    }
    current_widget_changed_str = new_widget_changed_str;
    if( id ) {
        ImGuiWindow *wnd = ImGui::GetCurrentWindow();
        current_widget_changed = wnd->GetID( id );
    }
    project_has_changes = true;
    edit_counter++;
}

bool HistoryState::has_unsaved_changes() const
{
    return !last_saved_snapshot || current_snapshot.num != *last_saved_snapshot;
}

bool HistoryState::has_unexported_changes() const
{
    return !last_exported_snapshot || current_snapshot.num != *last_exported_snapshot;
}

} // namespace editor
