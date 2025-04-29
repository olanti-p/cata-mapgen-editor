#include "project.h"

#include "common/timestamp.h"
#include "common/uuid.h"
#include <imgui/imgui.h>
#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "mapgen/palette_making.h"
#include "new_mapgen.h"
#include "new_palette.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"

#include <chrono>
#include <cstddef>
#include <memory>

namespace editor
{

const Mapgen *Project::get_mapgen( const UUID &fid ) const
{
    for( const Mapgen &mapgen : mapgens ) {
        if( fid == mapgen.uuid ) {
            return &mapgen;
        }
    }
    return nullptr;
}

const Palette *Project::get_palette( const UUID &fid ) const
{
    for( const Palette &palette : palettes ) {
        if( fid == palette.uuid ) {
            return &palette;
        }
    }
    return nullptr;
}

static bool show_palettes_tab(State& state, Project& project) {
    ImGui::Text( "Active Palettes:" );

    bool changed_palettes = ImGui::VectorWidget()
    .with_for_each( [&]( size_t idx ) {
        Palette &palette = project.palettes[idx];
        if( ImGui::Selectable( palette.display_name().c_str() ) ) {
            state.ui->toggle_show_palette_preview( palette.uuid );
        }
        ImGui::HelpPopup( "Click to show/hide palette preview." );
    } )
    .with_add( [&]()->bool {
        if (ImGui::Button( "Import palette..." ) )
        {
            state.ui->new_palette_window = std::make_unique<NewPaletteState>();
        }
        ImGui::SameLine();
        if( ImGui::Button( "New palette" ) )
        {
            quick_create_palette(state);
            return true;
        }
        return false;
    } )
    .with_can_delete( [&]( size_t idx ) {
        UUID uuid = project.palettes[idx].uuid;
        for( const Mapgen &mapgen : project.mapgens ) {
            if( mapgen.base.palette == uuid ) {
                return false;
            }
        }
        return true;
    } )
    .with_delete( [&]( size_t idx ) {
        project.palettes.erase( std::next( project.palettes.cbegin(), idx ) );
    } )
    /*
    * FIXME: implement palette duplication
    .with_duplicate( [&]( size_t idx ) {
        Palette copy = project.palettes[ idx ];
        copy.uuid = project.uuid_generator();
        project.palettes.insert( std::next( project.palettes.cbegin(), idx + 1 ), std::move( copy ) );
    } )
    */
    .with_default_move()
    .with_default_drag_drop()
    .run( project.palettes );

    return changed_palettes;
}

static bool show_mapgens_tab(State& state, Project& project) {
    ImGui::Text("Active Mapgens:");

    bool changed_mapgens = ImGui::VectorWidget()
        .with_for_each([&](size_t idx) {
        Mapgen& mapgen = project.mapgens[idx];
        if (ImGui::ImageButton("toggle_palette", "me_palette")) {
            state.ui->toggle_show_palette_preview(mapgen.base.palette);
        }
        ImGui::HelpPopup("Preview palette for this mapgen.");
        ImGui::SameLine();
        if (ImGui::ImageButton("toggle_mapobjects", "me_mapobject")) {
            state.ui->toggle_show_mapobjects(mapgen.uuid);
        }
        ImGui::HelpPopup("Show/hide map objects for this mapgen.");
        ImGui::SameLine();
        // FIXME: custom icon
        if (ImGui::ImageButton("toggle_setmaps", "me_set_rows_brush")) {
            state.ui->toggle_show_setmaps(mapgen.uuid);
        }
        ImGui::HelpPopup("Show/hide set points for this mapgen.");
        ImGui::SameLine();
        if (ImGui::Selectable(
            mapgen.display_name().c_str(),
            state.ui->active_mapgen_id && *state.ui->active_mapgen_id == mapgen.uuid)
            ) {
            state.ui->active_mapgen_id = mapgen.uuid;
        }
        ImGui::HelpPopup("Click to switch to this mapgen.");
            })
        .with_add([&]()->bool {
        if (ImGui::Button("Import mapgen..."))
        {
            state.ui->import_mapgen_window = std::make_unique<ImportMapgenState>();
        }
        ImGui::SameLine();
        if (ImGui::Button("New mapgen"))
        {
            state.ui->new_mapgen_window = std::make_unique<NewMapgenState>();
        }
        return false;
            })
        .with_delete([&](size_t idx) {
        project.mapgens.erase(std::next(project.mapgens.cbegin(), idx));
            })
        .with_duplicate([&](size_t idx) {
        Mapgen copy = project.mapgens[idx];
        copy.uuid = project.uuid_generator();
        project.mapgens.insert(std::next(project.mapgens.cbegin(), idx + 1), std::move(copy));
            })
        .with_default_move()
        .with_default_drag_drop()
        .run(project.mapgens);

    return changed_mapgens;
}

void show_project_overview_ui( State &state, Project &project, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 250.0f, 200.0f ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "Project Overview", &show );

    if (ImGui::BeginTabBar("ProjectContentsBar")) {
        bool changed = false;
        if (ImGui::BeginTabItem("Mapgens")) {
            changed = show_mapgens_tab(state, project) || changed;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Palettes")) {
            changed = show_palettes_tab(state, project) || changed;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
        if( changed ) {
            state.mark_changed();
        }
    }

    ImGui::End();
}

std::unique_ptr<Project> create_empty_project()
{
    auto ret = std::make_unique<Project>();
    ret->project_uuid = get_timestamp_ms_now();
    return ret;
}

const Palette* Project::find_palette_by_string(const std::string& id) const
{
    for (const Palette& p : palettes) {
        if (p.imported && p.imported_id.data == id) {
            return &p;
        }
        if (!p.imported && p.created_id == id) {
            return &p;
        }
    }
    return nullptr;
}

const Mapgen* Project::find_nested_mapgen_by_string(const std::string& id) const
{
    for (const Mapgen& it : mapgens) {
        if (it.mtype == MapgenType::Nested && it.nested.imported_mapgen_id == id) {
            return &it;
        }
    }
    return nullptr;
}

} // namespace editor
