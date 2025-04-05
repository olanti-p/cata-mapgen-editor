#include "new_mapgen.h"

#include "imgui.h"
#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "state/state.h"
#include "common/uuid.h"
#include "widget/widgets.h"
#include "state/ui_state.h"
#include "project.h"

#include <chrono>
#include <cstddef>

namespace editor
{
bool show_new_mapgen_window( State &state, NewMapgenState &mapgen )
{
    bool keep_open = true;
    ImGui::Begin( "New Mapgen", &keep_open );
    if( !keep_open ) {
        mapgen.cancelled = true;
    }

    ImGui::Text( "Mapgen type:" );
    if( ImGui::RadioButton( "Oter", mapgen.mtype == MapgenType::Oter ) ) {
        mapgen.mtype = MapgenType::Oter;
    }
    ImGui::HelpPopup(
        "Overmap terrain mapgen.\n\n"
        "Must be assigned to one (or more) overmap terrain types.\n"
        "When game generates local map for an OMT, it randomly selects one of the overmap mapgens associated "
        "with its type and runs it, then applies automatic transformations such as rotation.\n"
        "Each OMT type must have at least 1 OMT mapgen assigned to it."
    );
    ImGui::SameLine();
    if( ImGui::RadioButton( "Update", mapgen.mtype == MapgenType::Update ) ) {
        mapgen.mtype = MapgenType::Update;
    }
    ImGui::HelpPopup(
        "Update mapgen.\n\n"
        "Invoked by basecamp upgrade routines.\n"
        "Can be used for automatic calculation of camp blueprint requirements."
    );
    ImGui::SameLine();
    if( ImGui::RadioButton( "Nested", mapgen.mtype == MapgenType::Nested ) ) {
        mapgen.mtype = MapgenType::Nested;
    }
    ImGui::HelpPopup(
        "Nested mapgen.\n\n"
        "Can be invoked by all types of mapgen.\n"
        "This is essentially a 'chunk' of any size up to 24x24 that can be procedurally placed by the calling mapgen."
    );

    ImGui::Separator();

    ImGui::PaletteSelector( "Palette", mapgen.palette, state.project().palettes );

    ImGui::Separator();

    ImGui::InputText( "Name", &mapgen.name );
    ImGui::HelpPopup( "Display name.  Has no effect, just for convenience." );

    bool input_ok = (state.project().get_palette(mapgen.palette) != nullptr);

    ImGui::BeginDisabled( !input_ok );
    if( ImGui::Button( "Confirm" ) ) {
        mapgen.confirmed = true;
    }
    ImGui::EndDisabled();

    ImGui::End();

    if( mapgen.cancelled ) {
        return false;
    }
    if( mapgen.confirmed ) {
        add_mapgen( state, mapgen );
        state.mark_changed();
        return false;
    }
    return true;
}

void add_mapgen( State &state, NewMapgenState &mapgen )
{
    Project &project = state.project();
    UUID new_mapgen_uuid = project.uuid_generator();
    project.mapgens.emplace_back();
    Mapgen &new_mapgen = project.mapgens.back();
    new_mapgen.uuid = new_mapgen_uuid;
    new_mapgen.mtype = mapgen.mtype;
    new_mapgen.name = mapgen.name;
    new_mapgen.base.palette = mapgen.palette;
}

} // namespace editor
