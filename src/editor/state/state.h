#ifndef CATA_SRC_EDITOR_STATE_H
#define CATA_SRC_EDITOR_STATE_H

#include "pimpl.h"

#include <string>

namespace editor
{
struct HistoryState;
struct Project;
struct SaveExportState;
struct UiState;
struct ControlState;

struct State {
    State();
    explicit State( std::unique_ptr<Project> &&project );
    State( std::unique_ptr<Project> &&project, const std::string *loaded_from_path );
    State( const State & ) = delete;
    State( State && );
    ~State();

    State &operator=( const State & ) = delete;
    State &operator=( State && );

    pimpl<HistoryState> history;
    pimpl<SaveExportState> save_export;
    pimpl<ControlState> control;
    UiState *ui = nullptr;

    Project &project();

    /**
     * Mark project as changed.
     *
     * @param id (optional) If edit operation repeatedly generates change events that should be
     *           collapsed into a single undo/redo operation, pass id of the operation here.
     *           Respects current ImGui id stack.
     */
    void mark_changed( const char *id = nullptr );

    /**
     * Check whether project has been marked as changed.
     */
    bool is_changed() const;
};

/**
 * ============= Entry point =============
 */
void show_me_ui( State &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_STATE_H
