#ifndef CATA_SRC_EDITOR_HISTORY_STATE_H
#define CATA_SRC_EDITOR_HISTORY_STATE_H

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include <imgui/imgui.h>

namespace editor
{
struct Project;
struct ToolsState;
struct UiState;

using SnapshotNumber = int32_t;

struct ProjectSnapshot {
    std::unique_ptr<Project> project;
    SnapshotNumber num = 0;

    ProjectSnapshot();
    ProjectSnapshot( const ProjectSnapshot & ) = delete;
    ProjectSnapshot( ProjectSnapshot && );
    ~ProjectSnapshot();

    ProjectSnapshot &operator=( const ProjectSnapshot & ) = delete;
    ProjectSnapshot &operator=( ProjectSnapshot && );

    ProjectSnapshot make_copy() const;
};

struct HistoryState {
    HistoryState() = default;
    ~HistoryState() = default;
    HistoryState( std::unique_ptr<Project> &&project, bool was_loaded );

    HistoryState( const HistoryState & ) = delete;
    HistoryState( HistoryState && ) = default;
    HistoryState &operator=( const HistoryState & ) = delete;
    HistoryState &operator=( HistoryState && ) = default;

    inline Project &project() {
        return *current_snapshot.project;
    }

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
    inline bool is_changed() const {
        return project_has_changes;
    }

    inline bool can_undo() const {
        return current_snapshot.num != snapshots[snapshots.size() - 1].num;
    }

    inline void queue_undo() {
        switch_to_snapshot = current_snapshot.num - 1;
    }

    inline bool can_redo() const {
        return current_snapshot.num != snapshots[0].num;
    }

    inline void queue_redo() {
        switch_to_snapshot = current_snapshot.num + 1;
    }

    bool has_unsaved_changes() const;
    bool has_unexported_changes() const;

    bool project_has_changes = false;
    std::optional<ImGuiID> current_widget_changed = 0;
    std::string current_widget_changed_str;
    std::optional<ImGuiID> last_widget_changed = 0;
    std::optional<SnapshotNumber> switch_to_snapshot;
    ProjectSnapshot current_snapshot;
    std::vector<ProjectSnapshot> snapshots;
    int history_capacity = 200;
    std::optional<SnapshotNumber> last_saved_snapshot;
    std::optional<SnapshotNumber> last_exported_snapshot;
    std::optional<SnapshotNumber> last_autosaved_snapshot;
    int edit_counter = 0;
};

/**
 * =============== Windows ===============
 */
void show_edit_history( HistoryState &state, bool &show );

void handle_snapshot_change( HistoryState &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_HISTORY_STATE_H
