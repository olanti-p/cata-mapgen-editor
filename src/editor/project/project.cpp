#include "project.h"

#include "common/timestamp.h"
#include "common/uuid.h"
#include "imgui.h"
#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
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

void show_project_overview_ui( State &state, Project &project, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 250.0f, 200.0f ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "Project Overview", &show );

    ImGui::PushID( "palettes" );
    ImGui::Text( "Palettes:" );

    bool changed_palettes = ImGui::VectorWidget()
    .with_for_each( [&]( size_t idx ) {
        Palette &palette = project.palettes[idx];
        if( ImGui::ImageButton( "toggle_palette", "me_palette" ) ) {
            state.ui->toggle_show_palette_simple( palette.uuid );
        }
        ImGui::HelpPopup( "Show/hide palette." );
        ImGui::SameLine();
        if( ImGui::Selectable( palette.display_name().c_str() ) ) {
            state.ui->toggle_show_palette_verbose( palette.uuid );
        }
    } )
    .with_add( [&]()->bool {
        if( ImGui::Button( "New palette" ) )
        {
            state.ui->new_palette_window = std::make_unique<NewPaletteState>();
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
    .with_duplicate( [&]( size_t idx ) {
        Palette copy = project.palettes[ idx ];
        copy.uuid = project.uuid_generator();
        project.palettes.insert( std::next( project.palettes.cbegin(), idx + 1 ), std::move( copy ) );
    } )
    .with_default_move()
    .with_default_drag_drop()
    .run( project.palettes );

    ImGui::PopID();
    ImGui::Separator();
    ImGui::PushID("mapgens");
    ImGui::Text("Mapgens:");

    bool changed_mapgens = ImGui::VectorWidget()
        .with_for_each([&](size_t idx) {
        Mapgen& mapgen = project.mapgens[idx];
        if (ImGui::ImageButton("toggle_palette", "me_palette")) {
            state.ui->toggle_show_palette_simple(mapgen.base.palette);
        }
        ImGui::HelpPopup("Show/hide palette for this mapgen.");
        ImGui::SameLine();
        if (ImGui::ImageButton("toggle_mapobjects", "me_mapobject")) {
            state.ui->toggle_show_mapobjects(mapgen.uuid);
        }
        ImGui::HelpPopup("Show/hide map objects for this mapgen.");
        ImGui::SameLine();
        if (ImGui::Selectable(
            mapgen.display_name().c_str(),
            state.ui->active_mapgen_id && *state.ui->active_mapgen_id == mapgen.uuid)
            ) {
            state.ui->active_mapgen_id = mapgen.uuid;
        }
            })
        .with_add([&]()->bool {
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

    ImGui::PopID();

    if( changed_mapgens || changed_palettes ) {
        state.mark_changed();
    }

    ImGui::End();
}

std::unique_ptr<Project> create_empty_project()
{
    auto ret = std::make_unique<Project>();
    ret->project_uuid = get_timestamp_ms_now();
    return ret;
}

} // namespace editor
