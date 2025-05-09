#ifndef CATA_SRC_EDITOR_UI_STATE_H
#define CATA_SRC_EDITOR_UI_STATE_H

#include "pimpl.h"
#include "hash_utils.h"

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
struct ImportMapgenState;

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
    MapKey uuid;
    bool open = true;
};

struct ExpandedPiece {
    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonObject& jsin);

    UUID uuid = UUID_INVALID;
    UUID palette = UUID_INVALID;

    bool operator<(const ExpandedPiece& rhs) const {
        if (uuid < rhs.uuid) {
            return true;
        } else if (uuid==rhs.uuid) {
            return palette < rhs.palette;
        } else {
            return false;
        }
    }
};

struct OpenMapgenObject {
    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonObject&jsin );

    UUID uuid = UUID_INVALID;
    bool open = true;
};

struct OpenLootDesigner {
public:
    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonObject&jsin );

    bool is_mapping_mode = false;
    
    bool resolved = false;
    UUID palette = UUID_INVALID;
    MapKey map_key;

    UUID mapgen = UUID_INVALID;
    UUID mapgen_object = UUID_INVALID;


    std::string make_window_id() const;

    bool open = true;

    bool same_data_as(const OpenLootDesigner& rhs) const {
        return is_mapping_mode == rhs.is_mapping_mode &&
            resolved == rhs.resolved &&
            palette == rhs.palette &&
            map_key == rhs.map_key &&
            mapgen == rhs.mapgen &&
            mapgen_object == rhs.mapgen_object;
    }

    std::map<UUID, bool> enabled_pieces;
    bool has_been_copied = false;

    const std::string& get_display_cache() const {
        return display_cache;
    }
    std::string& get_display_cache() {
        return display_cache;
    }

    void set_display_cache( const std::string& s) {
        display_cache = s;
        has_been_copied = false;
    }
    
private:
    std::string display_cache;
};

} // namespace detail

struct ViewPaletteTreeState {
    std::unordered_map<UUID, std::vector<int>> selected_opts;
};

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
    bool show_mapgen_objects = true;        // Whether to show mapgen objects
    bool show_mapgen_setmaps = true;        // Whether to show mapgen objects
    bool show_history = true;               // Whether to show undo/redo history
    bool show_camera_controls = true;       // Whether to show camera controls
    bool show_toolbar = true;               // Whether to show canvas toolbar
    bool show_autosave_params = true;       // Whether to show autosave settings

    bool warn_on_import_issues = true;      // Whether to warn when import concludes with issues
    bool show_omt_grid = true;              // Whether to show omt grid on canvas
    bool show_canvas_symbols = true;        // Whether to show symbols on canvas
    bool show_canvas_sprites = true;        // Whether to show sprites on canvas
    bool show_canvas_objects = true;        // Whether to show objects on canvas
    bool show_canvas_setmaps = true;        // Whether to show setmaps on canvas
    bool show_fill_ter_sprites = true;      // Whether to show fill_ter sprites as fallback
    float canvas_sprite_opacity = 0.6f;     // Opacity for sprites on canvas

    bool autosave_enabled = true;
    int autosave_interval = 15;             // Seconds
    int autosave_limit = 30;                // Amount of autosaves to keep

    std::optional<UUID> active_mapgen_id;   // UUID of active mapgen

    std::vector<detail::OpenPalette> open_palette_previews; // List of open palettes (verbose)
    std::vector<detail::OpenMapping> open_source_mappings; // List of open mappings (in source mode)
    std::vector<detail::OpenMapping> open_resolved_mappings; // List of open mappings (in resolved mode)
    std::vector<detail::OpenMapgenObject> open_mapgenobjects; // List of open mapgenobjects
    std::vector<detail::OpenMapgenObject> open_setmaps; // List of open setpoints
    std::unordered_map<UUID, ViewPaletteTreeState> view_palette_tree_states;

    std::vector<detail::OpenLootDesigner> open_loot_designers; // List of open item spawn views (for mappings)

    pimpl<Camera> camera;
    pimpl<ToolsState> tools;

    std::set<detail::ExpandedPiece> expanded_pieces_source;
    std::set<detail::ExpandedPiece> expanded_pieces_resolved;
    std::set<UUID> expanded_mapobjects;
    std::set<UUID> expanded_setmaps;

    // TODO: move to control state?
    std::unique_ptr<NewMapgenState> new_mapgen_window;
    std::unique_ptr<ImportMapgenState> import_mapgen_window;
    std::unique_ptr<NewPaletteState> new_palette_window;

    std::optional<std::string> project_export_path;

    void toggle_show_palette_preview( UUID uuid );
    void toggle_show_source_mapping( UUID palette, MapKey uuid );
    void toggle_show_resolved_mapping(UUID palette, MapKey uuid);
    void toggle_show_mapobjects( UUID uuid );
    void toggle_show_setmaps(UUID uuid);

    void toggle_loot_designer_source_mappping(UUID palette, MapKey map_key);
    void toggle_loot_designer_resolved_mappping(UUID palette, MapKey map_key);
    void toggle_loot_designer_map_object(UUID mapgen, UUID object);
};

/**
 * =============== Windows ===============
 */
void show_camera_controls( State &state, bool &show );
void run_ui_for_state( State &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_UI_STATE_H
