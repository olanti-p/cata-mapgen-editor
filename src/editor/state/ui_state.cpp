#include "ui_state.h"

#include <imgui/imgui.h>

#include "view/camera.h"
#include "view/view_canvas.h"
#include "view/mouse.h"
#include "control_state.h"
#include "history_state.h"
#include "mapgen/palette.h"
#include "mapgen/palette_view.h"
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
#include "mapgen/loot_designer.h"

namespace editor
{
std::string detail::OpenLootDesigner::make_window_id() const
{
    std::string ret;
    ret += is_mapping_mode ? "mapping-" : "object-";
    if (is_mapping_mode) {
        ret += resolved ? "resolved-" : "source-";
        ret += string_format("%d-%s", palette, map_key.str());
    }
    else {
        ret += string_format("%d-%d", mapgen, mapgen_object);
    }
    return ret;
}

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

void UiState::toggle_show_source_mapping( UUID palette, MapKey uuid )
{
    for( auto &it : open_source_mappings ) {
        if( it.uuid == uuid && it.palette == palette ) {
            it.open = false;
            return;
        }
    }
    open_source_mappings.emplace_back();
    open_source_mappings.back().uuid = uuid;
    open_source_mappings.back().palette = palette;
}

void UiState::toggle_show_resolved_mapping(UUID palette, MapKey uuid)
{
    for (auto& it : open_resolved_mappings) {
        if (it.uuid == uuid && it.palette == palette) {
            it.open = false;
            return;
        }
    }
    open_resolved_mappings.emplace_back();
    open_resolved_mappings.back().uuid = uuid;
    open_resolved_mappings.back().palette = palette;
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

static void toggle_loot_designer(std::vector<detail::OpenLootDesigner>& list, detail::OpenLootDesigner&& value) {
    for (auto& it : list) {
        if (it.open && it.same_data_as(value)) {
            it.open = false;
            return;
        }
    }
    value.open = true;
    list.emplace_back(std::move(value));
}

void UiState::toggle_loot_designer_source_mappping(UUID palette, MapKey map_key)
{
    detail::OpenLootDesigner v;
    v.is_mapping_mode = true;
    v.resolved = false;
    v.palette = palette;
    v.map_key = map_key;
    toggle_loot_designer(open_loot_designers, std::move(v));
}

void UiState::toggle_loot_designer_resolved_mappping(UUID palette, MapKey map_key)
{
    detail::OpenLootDesigner v;
    v.is_mapping_mode = true;
    v.resolved = true;
    v.palette = palette;
    v.map_key = map_key;
    toggle_loot_designer(open_loot_designers, std::move(v));
}

void UiState::toggle_loot_designer_map_object(UUID mapgen, UUID object)
{
    detail::OpenLootDesigner v;
    v.is_mapping_mode = false;
    v.mapgen = mapgen;
    v.mapgen_object = object;
    toggle_loot_designer(open_loot_designers, std::move(v));
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
            "- Drag the view with MMB (mouse wheel) to pan.\n"
            "- Scroll with the mouse wheel over the view to zoom.\n"
            "- Hold Ctrl to see summary of highlighted tile.\n"
            "- Hold Alt and scroll to cycle between mapgens.\n"
            "- Use widgets in this window to manually control zoom and position.\n"
            "\nIn canvas mode:\n"
            "- Press Alt+LMB on tile to pick it.\n"
            "- Press Alt+LMB outside bounds (or on empty tile) to dismiss picked tile."
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

template<typename T>
void erase_closed_windows(std::vector<T>& list) {
    for (auto it = list.cbegin(); it != list.cend(); ) {
        if (!it->open) {
            it = list.erase(it);
        }
        else {
            it++;
        }
    }
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
    if (uistate.show_mapgen_objects && active_mapgen) {
        show_mapobjects(state, *active_mapgen, true, uistate.show_mapgen_objects);
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
    for( auto &it : uistate.open_source_mappings ) {
        if( !it.open ) {
            continue;
        }
        Palette *pal = proj.get_palette( it.palette );
        if( pal ) {
            PaletteEntry *entry = pal->find_entry( it.uuid );
            if( entry ) {
                show_mapping_source( state, *pal, *entry, it.open );
            } else {
                it.open = false;
            }
        } else {
            it.open = false;
        }
    }
    for (auto& it : uistate.open_resolved_mappings) {
        if (!it.open) {
            continue;
        }
        Palette* pal = proj.get_palette(it.palette);
        if (pal) {
            ViewPalette vp(proj);
            ViewPaletteTreeState &vpts = state.ui->view_palette_tree_states[pal->uuid];
            // FIXME: avoid creating temp view palette
            vp.add_palette_recursive(*pal, vpts);
            vp.finalize();
            ViewEntry* entry = vp.find_entry(it.uuid);
            if (entry) {
                show_mapping_resolved(state, vp, *entry, it.open);
            }
            else {
                it.open = false;
            }
        }
        else {
            it.open = false;
        }
    }
    for( auto &it : uistate.open_mapgenobjects ) {
        if( !it.open ) {
            continue;
        }
        Mapgen *f = proj.get_mapgen( it.uuid );
        if( f ) {
            show_mapobjects( state, *f, false, it.open );
        } else {
            it.open = false;
        }
    }
    for (auto& it : uistate.open_loot_designers) {
        show_loot_designer(state, it);
    }
    for(
        auto it = uistate.open_palette_previews.cbegin();
        it != uistate.open_palette_previews.cend();
    ) {
        if( !it->open ) {
            for( auto &mit : uistate.open_source_mappings ) {
                if( mit.palette == it->uuid ) {
                    mit.open = false;
                }
            }
            // TODO: close resolved mappings
            it = uistate.open_palette_previews.erase( it );
        } else {
            it++;
        }
    }
    erase_closed_windows(uistate.open_source_mappings);
    erase_closed_windows(uistate.open_resolved_mappings);
    erase_closed_windows(uistate.open_mapgenobjects);
    erase_closed_windows(uistate.open_loot_designers);

    if( !control.quick_add_state.active ) {
        control.quick_add_state = QuickPaletteAddState();
    }

    if (control.reset_focus) {
        control.reset_focus = false;
        ImGui::SetWindowFocus("<editor_view>");
    }

    if( !state.control->has_ongoing_tool_operation() ) {
        handle_snapshot_change( *state.history );
    }

    state.control->handle_warning_popup();
}

} // namespace editor
