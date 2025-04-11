#include "ui_state.h"

#include "view/camera.h"
#include "view/view_canvas.h"
#include "view/mouse.h"
#include "control_state.h"
#include "history_state.h"
#include "imgui.h"
#include "mapgen/palette.h"
#include "mapgen/palette_window.h"
#include "mapgen/mapgen.h"
#include "project/menu_bar.h"
#include "project/project.h"
#include "save_export_state.h"
#include "state.h"
#include "tools_state.h"
#include "ui_state_store.h"
#include "uistate.h"
#include "widget/widgets.h"
#include "project/new_mapgen.h"
#include "project/new_palette.h"
#include "mapgen/palette_making.h"

namespace editor
{
UiState::UiState() = default;
UiState::UiState( UiState && ) = default;
UiState::~UiState() = default;
UiState &UiState::operator=( UiState && ) = default;

void UiState::toggle_show_palette_preview( UUID uuid )
{
    for( auto &it : open_palette_previews ) {
        if( it.uuid == uuid ) {
            it.open = false;
            return;
        }
    }
    open_palette_previews.emplace_back();
    open_palette_previews.back().uuid = uuid;
}

void UiState::toggle_show_mapping( UUID palette, MapKey uuid )
{
    for( auto &it : open_mappings ) {
        if( it.uuid == uuid && it.palette == palette ) {
            it.open = false;
            return;
        }
    }
    open_mappings.emplace_back();
    open_mappings.back().uuid = uuid;
    open_mappings.back().palette = palette;
}

void UiState::toggle_show_mapobjects( UUID uuid )
{
    for( auto &it : open_mapgenobjects ) {
        if( it.uuid == uuid ) {
            it.open = false;
            return;
        }
    }
    open_mapgenobjects.emplace_back();
    open_mapgenobjects.back().uuid = uuid;
}

void show_camera_controls( State &state, bool &show )
{
    UiState &uistate = *state.ui;

    ImGui::SetNextWindowSize( ImVec2( 230.0f, 140.0f ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "Camera Controls", &show );

    // Camera
    {
        ImGui::TextDisabled( "(?: Camera contols)" );
        ImGui::HelpPopup(
            "Camera controls:\n\n"
            "- Drag the view with RMB to pan.\n"
            "- Scroll with the mouse wheel over the view to zoom.\n"
            "- Hold Ctrl to see summary of highlighted tile.\n"
            "- Hold Alt to measure distance or area.\n"
            "- Hold Alt while scrolling to switch between mapgens.\n"
            "- Use widgets in this window to manually control zoom and position.\n"
            "\nIn canvas mode:\n"
            "- Press MMB (mouse wheel) on tile to pick it.\n"
            "- Press MMB outside bounds (or on empty tile) to dismiss picked tile."
        );
        ImGui::DragInt( "Zoom", &uistate.camera->scale, 0.2f, MIN_SCALE, MAX_SCALE );
        ImGui::DragPoint( "Pos", &uistate.camera->pos, 1.0f, -10000, 10000 );
    }

    // Mouse position
    {
        point_abs_screen screen_pos = get_mouse_pos();
        point_abs_etile etile_pos = get_mouse_tile_pos( *uistate.camera );
        ImGui::Text( "Mouse pos, px: %s", screen_pos.to_string().c_str() );
        ImGui::Text( "Mouse pos, tile: %s", etile_pos.to_string().c_str() );
    }

    ImGui::End();
}

void run_ui_for_state( State &state )
{
    if( !state.ui ) {
        state.ui = &get_uistate_for_project( state.project().project_uuid );
    }

    show_main_menu_bar( state );

    handle_project_autosave( state );
    handle_project_saving( state );
    handle_project_exporting( state );
    handle_project_exiting( state );

    Project &proj = state.project();

    UiState &uistate = *state.ui;
    ControlState &control = *state.control;

    Mapgen *active_mapgen = nullptr;
    if( uistate.active_mapgen_id ) {
        active_mapgen = proj.get_mapgen( *uistate.active_mapgen_id );
        if( !active_mapgen ) {
            uistate.active_mapgen_id.reset();
        }
    }
    control.quick_add_state.active = false;

    if (control.reimport_all_palettes) {
        control.reimport_all_palettes = false;
        for (Palette& pal : proj.palettes) {
            if (pal.imported) {
                reimport_palette( state, pal );
            }
        }
        // FIXME: this should be transparent to the user
        state.mark_changed("palette-reimport-on-load");
    }

    // TODO: multiple mapgens on same canvas
    show_editor_view( state, active_mapgen );
    handle_view_change( state );

    if( uistate.show_demo_wnd ) {
        ImGui::ShowDemoWindow( &uistate.show_demo_wnd );
    }
    if( uistate.show_metrics_wnd ) {
        ImGui::ShowMetricsWindow( &uistate.show_metrics_wnd );
    }
    if( uistate.show_mapgen_info && active_mapgen ) {
        show_mapgen_info( state, *active_mapgen, uistate.show_mapgen_info );
    }
    if( uistate.show_mapgen_palette_simple && active_mapgen ) {
        Palette* pal = proj.get_palette(active_mapgen->base.palette);
        if (pal) {
            show_active_palette_simple(state, *pal, uistate.show_mapgen_palette_simple, true);
        }
        else {
            uistate.show_mapgen_palette_simple = false;
        }
    }
    if( uistate.show_mapgen_palette_verbose && active_mapgen ) {
        Palette* pal = proj.get_palette(active_mapgen->base.palette);
        if (pal) {
            show_active_palette_details(state, *pal, uistate.show_mapgen_palette_verbose);
        }
        else {
            uistate.show_mapgen_palette_verbose = false;
        }
    }
    if( uistate.show_project_overview ) {
        show_project_overview_ui( state, proj, uistate.show_project_overview );
    }
    if( uistate.show_history ) {
        show_edit_history( *state.history, uistate.show_history );
    }
    if( uistate.show_toolbar ) {
        show_toolbar( state, uistate.show_toolbar );
    }
    if( uistate.show_camera_controls ) {
        show_camera_controls( state, uistate.show_camera_controls );
    }
    if( uistate.show_autosave_params ) {
        show_autosave_settings( uistate, uistate.show_autosave_params );
    }
    if( uistate.new_mapgen_window ) {
        if( !show_new_mapgen_window( state, *uistate.new_mapgen_window ) ) {
            uistate.new_mapgen_window.reset();
        }
    }
    if (uistate.import_mapgen_window) {
        if (!show_import_mapgen_window(state, *uistate.import_mapgen_window)) {
            uistate.import_mapgen_window.reset();
        }
    }
    if( uistate.new_palette_window ) {
        if( !show_new_palette_window( state, *uistate.new_palette_window ) ) {
            uistate.new_palette_window.reset();
        }
    }

    for( auto &it : uistate.open_palette_previews ) {
        if( !it.open ) {
            continue;
        }
        Palette *pal = proj.get_palette( it.uuid );
        if( pal ) {
            show_palette_preview( state, *pal, it.open );
        } else {
            it.open = false;
        }
    }
    for( auto &it : uistate.open_mappings ) {
        if( !it.open ) {
            continue;
        }
        Palette *pal = proj.get_palette( it.palette );
        if( pal ) {
            PaletteEntry *entry = pal->find_entry( it.uuid );
            if( entry ) {
                show_mapping( state, *pal, *entry, it.open );
            } else {
                it.open = false;
            }
        } else {
            it.open = false;
        }
    }
    for( auto &it : uistate.open_mapgenobjects ) {
        if( !it.open ) {
            continue;
        }
        Mapgen *f = proj.get_mapgen( it.uuid );
        if( f ) {
            show_mapobjects( state, *f, it.open );
        } else {
            it.open = false;
        }
    }
    for(
        auto it = uistate.open_palette_previews.cbegin();
        it != uistate.open_palette_previews.cend();
    ) {
        if( !it->open ) {
            for( auto &mit : uistate.open_mappings ) {
                if( mit.palette == it->uuid ) {
                    mit.open = false;
                }
            }
            it = uistate.open_palette_previews.erase( it );
        } else {
            it++;
        }
    }
    for( auto it = uistate.open_mappings.cbegin(); it != uistate.open_mappings.cend(); ) {
        if( !it->open ) {
            it = uistate.open_mappings.erase( it );
        } else {
            it++;
        }
    }
    for( auto it = uistate.open_mapgenobjects.cbegin(); it != uistate.open_mapgenobjects.cend(); ) {
        if( !it->open ) {
            it = uistate.open_mapgenobjects.erase( it );
        } else {
            it++;
        }
    }
    if( !control.quick_add_state.active ) {
        control.quick_add_state = QuickPaletteAddState();
    }

    if( !state.control->has_ongoing_tool_operation() ) {
        handle_snapshot_change( *state.history );
    }

    state.control->handle_warning_popup();
}

} // namespace editor
