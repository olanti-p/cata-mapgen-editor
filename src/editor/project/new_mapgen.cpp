#include "new_mapgen.h"

#include "common/color.h"
#include "imgui.h"
#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "state/state.h"
#include "common/uuid.h"
#include "widget/widgets.h"
#include "state/ui_state.h"
#include "project.h"
#include "mapgen/palette_making.h"

// FIXME: header conflict
#include "../../mapgen.h"

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

bool show_import_mapgen_window(State& state, ImportMapgenState& mapgen)
{
    bool keep_open = true;
    ImGui::Begin("Import Mapgen", &keep_open);
    if (!keep_open) {
        mapgen.cancelled = true;
    }

    ImGui::Text("Mapgen type:");
    ImGui::BeginDisabled(); // TODO: implement update and nested
    if (ImGui::RadioButton("Oter", mapgen.mtype == MapgenType::Oter)) {
        mapgen.mtype = MapgenType::Oter;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Update", mapgen.mtype == MapgenType::Update)) {
        mapgen.mtype = MapgenType::Update;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Nested", mapgen.mtype == MapgenType::Nested)) {
        mapgen.mtype = MapgenType::Nested;
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    bool input_ok = false;

    // FIXME: size math here is probably wrong
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetFrameHeight() );
    if (mapgen.mtype == MapgenType::Oter) {
        ImGui::InputId("##oter", mapgen.oter);
        input_ok = mapgen.oter.is_valid();
    } else if (mapgen.mtype == MapgenType::Nested) {
        ImGui::InputId("##nested", mapgen.nested);
        input_ok = mapgen.nested.is_valid();
    } else {
        ImGui::InputId("##update", mapgen.update);
        input_ok = mapgen.update.is_valid();
    }
    ImGui::PopItemWidth();

    ImGui::BeginDisabled(!input_ok);
    if (ImGui::Button("Confirm")) {
        mapgen.confirmed = true;
    }
    ImGui::EndDisabled();

    ImGui::End();

    if (mapgen.cancelled) {
        return false;
    }
    if (mapgen.confirmed) {
        Mapgen* new_mapgen = import_mapgen(state, mapgen);
        state.mark_changed();
        state.ui->active_mapgen_id = new_mapgen->uuid;
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

Mapgen* import_mapgen(State& state, ImportMapgenState& mapgen)
{
    Project& project = state.project();
    UUID new_mapgen_uuid = project.uuid_generator();
    project.mapgens.emplace_back();
    Mapgen& new_mapgen = project.mapgens.back();
    new_mapgen.uuid = new_mapgen_uuid;
    new_mapgen.mtype = mapgen.mtype;
    UUID new_palette = UUID_INVALID;

    if (mapgen.mtype == MapgenType::Oter) {
        mapgen_function_json* ref = editor_mapgen_refs[mapgen.oter.data];
        if (!ref->editor_mode) {
            // Shouldn't happen
            std::abort();
        }
        new_mapgen.name = mapgen.oter.data;
        new_mapgen.oter.weight = ref->editor_weight;
        auto &matrix = ref->editor_matrix;
        point ms_size(matrix[0].size(), matrix.size());
        point oter_size = ms_size / 24;
        if (oter_size.x == 0 || oter_size.y == 0) {
            std::abort();
        }
        if (oter_size != point(1, 1)) {
            new_mapgen.oter.matrix_mode = true;
            new_mapgen.oter.om_terrain_matrix.set_size(oter_size);
        }
        else {
            new_mapgen.oter.matrix_mode = false;
        }
        new_mapgen.set_canvas_size(ms_size);

        for (size_t y = 0; y < ms_size.y; y++) {
            for (size_t x = 0; x < ms_size.x; x++) {
                new_mapgen.base.canvas.set(point(x, y), matrix[y][x]);
            }
        }

        Palette* loaded_palette = state.project().find_palette_by_string(ref->editor_palette_id);
        if (!loaded_palette) {
            quick_import_temp_palette(state, EID::TempPalette(ref->editor_palette_id));
            loaded_palette = state.project().find_palette_by_string(ref->editor_palette_id);
            if (!loaded_palette) {
                std::abort();
            }
            recursive_import_palette(state, *loaded_palette);
            // FIXME: palettes need to be persistent in memory...
            loaded_palette = state.project().find_palette_by_string(ref->editor_palette_id);
        }
        loaded_palette->temp_palette = true;
        new_palette = loaded_palette->uuid;

        // TODO: parametric fill_ter
        auto fill_ter_original = ref->get_fill_ter();
        if (fill_ter_original) {
            new_mapgen.oter.fill_ter = fill_ter_original->id();
            if (!new_mapgen.oter.fill_ter.is_valid()) {
                // TODO: this shouldn't happen
                new_mapgen.oter.fill_ter = EID::Ter();
            }
        }

        if (new_mapgen.oter.matrix_mode) {
            for (size_t y = 0; y < oter_size.y; y++) {
                for (size_t x = 0; x < oter_size.x; x++) {
                    new_mapgen.oter.om_terrain_matrix.set(
                        point(x, y), EID::OterType(
                            ref->editor_oter_matrix[y][x]
                        )
                    );
                }
            }
        }
        else {
            for (const std::string& id : ref->editor_oter_list) {
                new_mapgen.oter.om_terrain.emplace_back(EID::OterType(id));
            }
        }

        PaletteImportReport report; // TODO: make use of this
        for (const auto& obj : ref->objects.objects) {
            const jmapgen_place& place = obj.first;
            const jmapgen_piece& piece = *obj.second.get();
            MapObject mo = MapObject();

            mo.piece = import_simple_piece(piece, report);
            mo.set_uuid(project.uuid_generator());
            mo.x = place.x;
            mo.y = place.y;
            // No z coord. Screw it.
            // TODO: maybe implement it later
            mo.repeat = place.repeat;
            // TODO: assign specific colors to types
            mo.color = ImColor::HSV(rng_float(0.0f, 1.0f), 1.0f, 1.0f, 0.6f);

            new_mapgen.objects.emplace_back(std::move(mo));
        }
    }
    else if (mapgen.mtype == MapgenType::Nested) {
        // TODO
    }
    else {
        // TODO
    }

    new_mapgen.base.palette = new_palette;
    return &new_mapgen;
}


} // namespace editor
