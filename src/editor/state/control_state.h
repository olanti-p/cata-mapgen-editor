#ifndef CATA_SRC_EDITOR_CONTROL_STATE_H
#define CATA_SRC_EDITOR_CONTROL_STATE_H

#include "common/uuid.h"
#include "tool/tool.h"
#include "view/ruler.h"
#include "widget/editable_id.h"
#include "mapgen/canvas_snippet.h"

#include <memory>
#include <unordered_map>

namespace editor
{
enum class QuickAddMode {
    Ter,
    Furn,
    Both,
};

struct QuickPaletteAddState {
    UUID palette = UUID_INVALID;
    bool active = false;
    QuickAddMode mode = QuickAddMode::Ter;
    EID::Ter eid_ter;
    EID::Furn eid_furn;
};

struct SnippetsState {
    public:
        std::optional<CanvasSnippet> clipboard;

        inline bool has_snippet_for( UUID mapgen ) const {
            return snippets.count( mapgen );
        }
        inline void add_snippet( UUID mapgen, CanvasSnippet &&new_snippet ) {
            snippets[mapgen] = std::move( new_snippet );
        }
        inline CanvasSnippet drop_snippet( UUID mapgen ) {
            auto it = snippets.find( mapgen );
            CanvasSnippet data = std::move( it->second );
            snippets.erase( it );
            return data;
        }
        inline CanvasSnippet *get_snippet( UUID mapgen ) {
            auto it = snippets.find( mapgen );
            if( it == snippets.end() ) {
                return nullptr;
            } else {
                return &it->second;
            }
        }

    private:
        std::unordered_map<UUID, CanvasSnippet> snippets;
};

/**
 * Editor control state.
 *
 * This handles control flow and window transitions.
 */
struct ControlState {
    public:
        ControlState();
        ControlState( const ControlState & ) = delete;
        ControlState( ControlState && );
        ~ControlState();

        ControlState &operator=( const ControlState & ) = delete;
        ControlState &operator=( ControlState && );

        bool is_editor_running = true;
        bool want_close = false;                // User wants to close the project
        bool want_save = false;                 // User wants to save
        bool want_save_as = false;              // User wants to save as
        bool want_exit_after_save = false;      // User wants to exit after save
        bool want_export = false;               // User wants to export
        bool want_export_as = false;            // User wants to export as
        int want_change_view = 0;               // User wants to change view
        bool reimport_all_palettes = false;     // Reimport palettes, for loadede projects that use imports

        map_key highlight_entry_in_palette;

        RulerState ruler;

        bool has_ongoing_tool_operation();
        tools::ToolControl &get_tool_control( tools::ToolKind t );

        QuickPaletteAddState quick_add_state;
        SnippetsState snippets;

        void show_warning_popup( const std::string &data );
        void handle_warning_popup();
        void handle_import_rows_from_clipboard();

    private:
        void set_tool_control( tools::ToolKind t );
        std::unique_ptr<tools::ToolControl> tool_control;
        tools::ToolKind tool_control_kind = tools::ToolKind::Cursor;

        bool want_show_warning_popup = false;
        std::string warning_popup_data;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_CONTROL_STATE_H
