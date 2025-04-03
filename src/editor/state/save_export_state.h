#ifndef CATA_SRC_EDITOR_SAVE_EXPORT_STATE_H
#define CATA_SRC_EDITOR_SAVE_EXPORT_STATE_H

#include <optional>
#include <string>

namespace editor
{
struct State;
struct UiState;

struct SaveExportState {
    SaveExportState() = default;
    ~SaveExportState() = default;

    SaveExportState( const SaveExportState & ) = delete;
    SaveExportState( SaveExportState && ) = default;
    SaveExportState &operator=( const SaveExportState & ) = delete;
    SaveExportState &operator=( SaveExportState && ) = default;

    std::optional<std::string> project_save_path;
    float elapsed_since_autosave = 0.0f;
};

void handle_project_autosave( State &state );
void handle_project_saving( State &state );
void handle_project_exporting( State &state );
void handle_project_exiting( State &state );
void show_autosave_settings( UiState &ui, bool &show );

} // namespace editor

#endif // CATA_SRC_EDITOR_SAVE_EXPORT_STATE_H
