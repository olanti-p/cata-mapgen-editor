#include "new_mapgen.h"

#include "common/color.h"
#include <imgui/imgui.h>
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

static void choose_mapgen_id( std::string& chosen, bool&input_ok, const std::vector<std::string>& opts) {
    auto chosen_it = std::find(opts.begin(), opts.end(), chosen);
    int chosen_idx = chosen_it == opts.end() ? -1 : static_cast<int>( std::distance(opts.begin(), chosen_it) );

    ImGui::ComboWithFilter("##choice", &chosen_idx, opts, 16);

    if (chosen_idx >= 0) {
        chosen = opts[chosen_idx];
        input_ok = true;
    }
    else {
        input_ok = false;
    }
}

bool show_import_mapgen_window(State& state, ImportMapgenState& mapgen)
{
    bool keep_open = true;
    ImGui::Begin("Import Mapgen", &keep_open);
    if (!keep_open) {
        mapgen.cancelled = true;
    }

    ImGui::Text("Mapgen type:");
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

    ImGui::Separator();

    bool input_ok = false;

    // FIXME: size math here is probably wrong
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - ImGui::GetFrameHeight() );
    if (mapgen.mtype == MapgenType::Oter) {
        std::vector<std::string> opts;
        for (const auto& it : editor_mapgen_refs) {
            opts.push_back(it.first);
        }
        choose_mapgen_id(mapgen.oter, input_ok, opts);
    } else if (mapgen.mtype == MapgenType::Nested) {
        std::vector<std::string> opts;
        for (const auto& it : editor_mapgen_refs_nested) {
            opts.push_back(it.first);
        }
        choose_mapgen_id(mapgen.nested, input_ok, opts);
    } else {
        std::vector<std::string> opts;
        for (const auto& it : editor_mapgen_refs_update) {
            opts.push_back(it.first);
        }
        choose_mapgen_id(mapgen.update, input_ok, opts);
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

static void import_common(State& state, Mapgen& mapgen, Project&project, mapgen_function_json_base& ref) {
    Palette* loaded_palette = project.find_palette_by_string(ref.editor_palette_id);
    if (!loaded_palette) {
        quick_import_temp_palette(state, EID::TempPalette(ref.editor_palette_id));
        loaded_palette = state.project().find_palette_by_string(ref.editor_palette_id);
        if (!loaded_palette) {
            std::abort();
        }
        recursive_import_palette(state, *loaded_palette);
        // FIXME: palettes need to be persistent in memory...
        loaded_palette = state.project().find_palette_by_string(ref.editor_palette_id);
    }
    loaded_palette->temp_palette = true;
    mapgen.base.palette = loaded_palette->uuid;


    auto& matrix = ref.editor_matrix;
    point ms_size(matrix[0].size(), matrix.size());
    mapgen.set_canvas_size(ms_size);

    for (size_t y = 0; y < ms_size.y; y++) {
        for (size_t x = 0; x < ms_size.x; x++) {
            mapgen.base.canvas.set(point(x, y), MapKey(matrix[y][x]));
        }
    }


    mapgen.flags.avoid_creatures = ref.flags_.test(jmapgen_flags::avoid_creatures);
    mapgen.flags.no_underlying_rotate = ref.flags_.test(jmapgen_flags::no_underlying_rotate);
    if (ref.flags_.test(jmapgen_flags::allow_terrain_under_other_data)) {
        mapgen.flags.rule_items = LayeringRuleItems::Allow;
        mapgen.flags.rule_furn = LayeringRuleTrapFurn::Allow;
        mapgen.flags.rule_traps = LayeringRuleTrapFurn::Allow;
    }
    else if (ref.flags_.test(jmapgen_flags::dismantle_all_before_placing_terrain)) {
        mapgen.flags.rule_items = LayeringRuleItems::Allow;
        mapgen.flags.rule_furn = LayeringRuleTrapFurn::Dismantle;
        mapgen.flags.rule_traps = LayeringRuleTrapFurn::Dismantle;
    }
    else if (ref.flags_.test(jmapgen_flags::erase_all_before_placing_terrain)) {
        mapgen.flags.rule_items = LayeringRuleItems::Erase;
        mapgen.flags.rule_furn = LayeringRuleTrapFurn::Erase;
        mapgen.flags.rule_traps = LayeringRuleTrapFurn::Erase;
    }
    else {
        if (ref.flags_.test(jmapgen_flags::allow_terrain_under_items)) {
            mapgen.flags.rule_items = LayeringRuleItems::Allow;
        }
        else if (ref.flags_.test(jmapgen_flags::erase_items_before_placing_terrain)) {
            mapgen.flags.rule_items = LayeringRuleItems::Erase;
        }

        if (ref.flags_.test(jmapgen_flags::allow_terrain_under_furniture)) {
            mapgen.flags.rule_furn = LayeringRuleTrapFurn::Allow;
        }
        else if (ref.flags_.test(jmapgen_flags::dismantle_furniture_before_placing_terrain)) {
            mapgen.flags.rule_furn = LayeringRuleTrapFurn::Dismantle;
        }
        else if (ref.flags_.test(jmapgen_flags::erase_furniture_before_placing_terrain)) {
            mapgen.flags.rule_furn = LayeringRuleTrapFurn::Erase;
        }

        if (ref.flags_.test(jmapgen_flags::allow_terrain_under_trap)) {
            mapgen.flags.rule_traps = LayeringRuleTrapFurn::Allow;
        }
        else if (ref.flags_.test(jmapgen_flags::dismantle_trap_before_placing_terrain)) {
            mapgen.flags.rule_traps = LayeringRuleTrapFurn::Dismantle;
        }
        else if (ref.flags_.test(jmapgen_flags::erase_trap_before_placing_terrain)) {
            mapgen.flags.rule_traps = LayeringRuleTrapFurn::Erase;
        }
    }


    PaletteImportReport report; // TODO: make use of this
    int color_counter = 0;
    int num_colors = 19;
    int color_step = 11;
    for (const auto& obj : ref.objects.objects) {
        const jmapgen_place& place = obj.first;
        const jmapgen_piece& piece = *obj.second.get();
        MapObject mo = MapObject();

        mo.piece = import_simple_piece(piece, report, true);
        mo.piece->is_object = true;
        mo.set_uuid(project.uuid_generator());
        mo.x = place.x;
        mo.y = place.y;
        // No z coord. Screw it.
        // TODO: maybe implement it later
        mo.repeat = place.repeat;
        // TODO: assign specific colors to types
        int color_fraction = (color_counter * color_step) % num_colors;
        float h = float(color_fraction) / float(num_colors);
        color_counter++;
        mo.color = ImColor::HSV(h, 1.0f, 1.0f, 0.6f);

        mapgen.objects.emplace_back(std::move(mo));
    }
}

Mapgen* import_mapgen(State& state, ImportMapgenState& mapgen)
{
    Project& project = state.project();
    UUID new_mapgen_uuid = project.uuid_generator();
    project.mapgens.emplace_back();
    Mapgen& new_mapgen = project.mapgens.back();
    new_mapgen.uuid = new_mapgen_uuid;
    new_mapgen.mtype = mapgen.mtype;

    if (mapgen.mtype == MapgenType::Oter) {
        new_mapgen.mtype = MapgenType::Oter;
        new_mapgen.name = mapgen.oter;

        mapgen_function_json* ref = editor_mapgen_refs[mapgen.oter];
        if (!ref->editor_mode) {
            // Shouldn't happen
            std::abort();
        }

        import_common(state, new_mapgen, project, *ref);

        new_mapgen.oter.weight = ref->editor_weight;
        new_mapgen.oter.rotation = ref->rotation;

        // TODO: parametric fill_ter
        auto fill_ter_original = ref->get_fill_ter();
        if (fill_ter_original) {
            new_mapgen.oter.mapgen_base = OterMapgenFill::FillTer;
            new_mapgen.oter.fill_ter = fill_ter_original->id();
            if (!new_mapgen.oter.fill_ter.is_valid()) {
                // TODO: this shouldn't happen
                new_mapgen.oter.fill_ter = EID::Ter();
            }
        } else if (ref->predecessor_mapgen.id() != oter_str_id::NULL_ID()) {
            new_mapgen.oter.mapgen_base = OterMapgenFill::PredecessorMapgen;
            new_mapgen.oter.predecessor_mapgen = EID::OterType( ref->predecessor_mapgen.id().str() );
        } else if (ref->fallback_predecessor_mapgen_ != oter_str_id::NULL_ID()) {
            new_mapgen.oter.mapgen_base = OterMapgenFill::FallbackPredecessorMapgen;
            new_mapgen.oter.predecessor_mapgen = EID::OterType(ref->fallback_predecessor_mapgen_.str());
        }

        point oter_size = new_mapgen.base.canvas.get_size() / 24;
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
    }
    else if (mapgen.mtype == MapgenType::Nested) {
        new_mapgen.mtype = MapgenType::Nested;
        new_mapgen.name = mapgen.nested;

        mapgen_function_json_nested* ref = editor_mapgen_refs_nested[mapgen.nested];
        if (!ref->editor_mode) {
            // Shouldn't happen
            std::abort();
        }

        import_common(state, new_mapgen, project, *ref);

        new_mapgen.nested.size = new_mapgen.base.canvas.get_size();
        new_mapgen.nested.rotation = ref->rotation;
        new_mapgen.nested.nested_mapgen_id = ref->editor_mapgen_id;

        // TODO
    }
    else {
        new_mapgen.mtype = MapgenType::Update;
        new_mapgen.name = mapgen.update;

        update_mapgen_function_json* ref = editor_mapgen_refs_update[mapgen.update];
        if (!ref->editor_mode) {
            // Shouldn't happen
            std::abort();
        }

        import_common(state, new_mapgen, project, *ref);

        new_mapgen.update.uses_rows = !ref->editor_matrix.empty();
        new_mapgen.update.update_mapgen_id = ref->editor_mapgen_id;

        // TODO
    }

    return &new_mapgen;
}


} // namespace editor
