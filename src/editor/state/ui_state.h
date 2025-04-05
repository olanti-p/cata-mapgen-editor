#ifndef CATA_SRC_EDITOR_UI_STATE_H
#define CATA_SRC_EDITOR_UI_STATE_H

#include "pimpl.h"

#include "common/uuid.h"
#include "view/camera.h"
#include "state/tools_state.h"

#include <set>

namespace editor
{
struct State;
struct Camera;
struct ToolsState;
struct NewMapgenState;
struct NewPaletteState;

namespace detail
{
struct OpenPalette {
    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonObject &jsin );

    UUID uuid = UUID_INVALID;
    bool open = true;
};

struct OpenMapping {
    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonObject&jsin );

    UUID palette = UUID_INVALID;
    map_key uuid;
    bool open = true;
};

struct OpenMapgenObject {
    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonObject&jsin );

    UUID uuid = UUID_INVALID;
    bool open = true;
};
} // namespace detail

/**
 * Editor UI state.
 *
 * It's saving and loading is managed by imgui settings save/load routines,
 * so it may fall out of sync with the project state (e.g. use nonexistent uuids).
 *
 * As such, it is required to check members for validity when using them,
 * and in case of invalidation - reset to some neutral but valid state.
*/
struct UiState {
    UiState();
    UiState( const UiState & ) = delete;
    UiState( UiState && );
    ~UiState();

    UiState &operator=( const UiState & ) = delete;
    UiState &operator=( UiState && );

    void serialize( JsonOut &jsout ) const;
    void deserialize( JSON_IN &jsin );

    bool show_demo_wnd = false;             // Whether to show ImGui Demo window
    bool show_metrics_wnd = false;          // Whether to show ImGui Metrics/Debugger window
    bool show_project_overview = true;      // Whether to show project overview window
    bool show_mapgen_info = true;           // Whether to show mapgen info
    bool show_mapgen_palette_simple = true; // Whether to show mapgen palette (simple)
    bool show_mapgen_palette_verbose = true;// Whether to show mapgen palette (verbose)
    bool show_history = true;               // Whether to show undo/redo history
    bool show_camera_controls = true;       // Whether to show camera controls
    bool show_toolbar = true;               // Whether to show canvas toolbar
    bool show_autosave_params = true;       // Whether to show autosave settings

    bool show_omt_grid = false;             // Whether to show omt grid on canvas
    bool show_canvas_symbols = false;       // Whether to show symbols on canvas
    bool show_canvas_sprites = false;       // Whether to show sprites on canvas
    float canvas_sprite_opacity = 0.4f;     // Opacity for sprites on canvas

    bool autosave_enabled = true;
    int autosave_interval = 10;             // Seconds
    int autosave_limit = 5;                 // Amount of autosaves to keep

    std::optional<UUID> active_mapgen_id;   // UUID of active mapgen

    std::vector<detail::OpenPalette> open_palette_previews; // List of open palettes (verbose)
    std::vector<detail::OpenMapping> open_mappings; // List of open mappings
    std::vector<detail::OpenMapgenObject> open_mapgenobjects; // List of open mapgenobjects

    pimpl<Camera> camera;
    pimpl<ToolsState> tools;

    std::set<UUID> expanded_mapping_pieces;
    std::set<UUID> expanded_mapobjects;

    std::unique_ptr<NewMapgenState> new_mapgen_window;
    std::unique_ptr<NewPaletteState> new_palette_window;

    std::optional<std::string> project_export_path;

    void toggle_show_palette_preview( UUID uuid );
    void toggle_show_mapping( UUID palette, map_key uuid );
    void toggle_show_mapobjects( UUID uuid );
};

/**
 * =============== Windows ===============
 */
void show_camera_controls( State &state, bool &show );
void run_ui_for_state( State &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_UI_STATE_H
