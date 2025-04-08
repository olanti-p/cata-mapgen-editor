#include "piece_impl.h"

#include "common/algo.h"
#include "widget/style.h"
#include "state/state.h"
#include "widget/widgets.h"

// FIXME: header name conflict
#include "../../mapgen.h"

namespace editor
{
template<typename T>
void show_weighted_list(State& state, editor::WeightedList<T>& list)
{
    ImGui::Indent(style::list_indent);

    const auto can_delete = [&](size_t) -> bool {
        return list.entries.size() > 1;
        };

    const auto show_val = [&](size_t i) {
        ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 5.0f);
        bool bad_weight = list.entries[i].weight <= 0;
        if (bad_weight) {
            ImGui::BeginErrorArea();
        }
        if (ImGui::InputInt("##weight", &list.entries[i].weight)) {
            state.mark_changed("entry-weight");
        }
        if (bad_weight) {
            ImGui::EndErrorArea();
        }
        ImGui::HelpPopup("Weight");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 10.0f);
        if (ImGui::InputId("##ter", list.entries[i].val)) {
            state.mark_changed();
        }
        ImGui::HelpPopup("Value");
        };

    if (
        ImGui::VectorWidget()
        .with_for_each(show_val)
        .with_can_delete(can_delete)
        .with_default_add()
        .with_default_delete()
        .with_default_move()
        .with_default_drag_drop()
        .run(list.entries)) {
        state.mark_changed();
    }

    ImGui::Indent(-style::list_indent);
}

void PieceField::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Type of the field." );
    if( ImGui::InputId( "ftype", ftype ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Intensity of the field." );
    if( ImGui::InputIntClamped( "intensity", intensity, 1, 3 ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Age of the field at the moment of spawn.  Affects decay rate." );
    if( ImGui::InputDuration( "age", age ) ) {
        state.mark_changed();
    }
}

std::string PieceField::fmt_data_summary() const
{
    return string_format( "%s:%d", ftype.data, intensity );
}

void PieceNPC::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "NPC template to use." );
    if( ImGui::InputId( "npc_class", npc_class ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Whether this NPC is the target of a quest." );
    if( ImGui::Checkbox( "target", &target ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "List of additional character traits applied on spawn." );
    ImGui::Text( "traits:" );
    ImGui::Indent( style::list_indent );
    bool changed = ImGui::VectorWidget()
    .with_for_each( [&]( size_t idx ) {
        if( ImGui::InputId( "##trait-input", traits[idx] ) ) {
            state.mark_changed();
        }
    } )
    .with_default_add()
    .with_default_delete()
    .with_default_move()
    .with_default_drag_drop()
    .run( traits );
    if( changed ) {
        state.mark_changed();
    }
    ImGui::Indent( -style::list_indent );
}

std::string PieceNPC::fmt_data_summary() const
{
    return npc_class.data;
}

void PieceFaction::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Faction id string.\n\nWARNING: no validation is done here." );
    // TODO: validation
    if( ImGui::InputText( "id", &id ) ) {
        state.mark_changed( "me-piece-faction-id-input" );
    }
}

std::string PieceFaction::fmt_data_summary() const
{
    return id;
}

static void sign_or_graffiti(
    State &state,
    bool is_sign,
    bool &use_snippet,
    EID::SnippetCategory &snippet,
    std::string &text
)
{
    ImGui::HelpMarkerInline( "Signs and graffiti can use either exact text string or a random snippet from category." );
    if( ImGui::Checkbox( "Use snippet from category", &use_snippet ) ) {
        state.mark_changed();
    }
    if( !use_snippet ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Snippet category to draw from." );
    if( ImGui::InputId( "snippet", snippet ) ) {
        state.mark_changed();
    }
    if( !use_snippet ) {
        ImGui::EndDisabled();
    } else {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Exact text to use." );
    if( ImGui::InputText( "text", &text ) ) {
        state.mark_changed( is_sign ? "me-piece-sign-text-input" : "me-piece-graffiti-text-input" );
    }
    if( use_snippet ) {
        ImGui::EndDisabled();
    }
}

void PieceSign::show_ui( State &state )
{
    sign_or_graffiti( state, true, use_snippet, snippet, text );
}

std::string PieceSign::fmt_data_summary() const
{
    if( use_snippet ) {
        return string_format( "<%s>", snippet.data );
    } else {
        return string_format( "\"%s\"", text );
    }
}

void PieceGraffiti::show_ui( State &state )
{
    sign_or_graffiti( state, false, use_snippet, snippet, text );
}

std::string PieceGraffiti::fmt_data_summary() const
{
    if( use_snippet ) {
        return string_format( "<%s>", snippet.data );
    } else {
        return string_format( "\"%s\"", text );
    }
}

void PieceVendingMachine::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Whether this vending machine is reinforced." );
    if( ImGui::Checkbox( "reinforced", &reinforced ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Whether to use \"default_vending_machine\" group." );
    if( ImGui::Checkbox( "Use default item group", &use_default_group ) ) {
        state.mark_changed();
    }
    if( use_default_group ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Item group to spawn items from." );
    if( ImGui::InputId( "item_group", item_group ) ) {
        state.mark_changed();
    }
    if( use_default_group ) {
        ImGui::EndDisabled();
    }
}

std::string PieceVendingMachine::fmt_data_summary() const
{
    if( use_default_group ) {
        return "default_vending_machine";
    } else {
        return item_group.data;
    }
}

void PieceToilet::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Whether to use default amount (24)." );
    if( ImGui::Checkbox( "Use default amount", &use_default_amount ) ) {
        state.mark_changed();
    }
    if( use_default_amount ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Amount of water to spawn, [min, max]." );
    if( ImGui::InputIntRange( "amount", amount ) ) {
        state.mark_changed( "me-piece-toilet-amount-input" );
    }
    if( use_default_amount ) {
        ImGui::EndDisabled();
    }
}

std::string PieceToilet::fmt_data_summary() const
{
    return "TODO";
}

void PieceGaspump::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Whether to use default amount\n( random value within [10000, 50000] )." );
    if( ImGui::Checkbox( "Use default amount", &use_default_amount ) ) {
        state.mark_changed();
    }
    if( use_default_amount ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Amount of fuel to spawn, [min, max]." );
    if( ImGui::InputIntRange( "amount", amount ) ) {
        state.mark_changed( "me-piece-gaspump-amount-input" );
    }
    if( use_default_amount ) {
        ImGui::EndDisabled();
    }

    ImGui::HelpMarkerInline(
        "Type of fuel to spawn.\n\n"
        "Setting it to 'Random' will choose randomly between diesel (25% chance) and gasoline (75% chance)."
    );
    ImGui::Text( "Fuel type:" );
    if( ImGui::RadioButton( "Random", fuel == GasPumpFuel::Random ) ) {
        fuel = GasPumpFuel::Random;
    }
    if( ImGui::RadioButton( "Diesel", fuel == GasPumpFuel::Diesel ) ) {
        fuel = GasPumpFuel::Diesel;
    }
    if( ImGui::RadioButton( "Gasoline", fuel == GasPumpFuel::Gasoline ) ) {
        fuel = GasPumpFuel::Gasoline;
    }
}

std::string PieceGaspump::fmt_data_summary() const
{
    return "TODO";
}

void PieceLiquid::show_ui( State &state )
{
    // TODO: show default amount from item
    ImGui::HelpMarkerInline( "Whether to use default amount\n( derived from item type )." );
    if( ImGui::Checkbox( "Use default amount", &use_default_amount ) ) {
        state.mark_changed();
    }
    if( use_default_amount ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline( "Amount of liquid to spawn, [min, max]." );
    if( ImGui::InputIntRange( "amount", amount ) ) {
        state.mark_changed( "me-piece-liquid-amount-input" );
    }
    if( use_default_amount ) {
        ImGui::EndDisabled();
    }

    ImGui::HelpMarkerInline( "Type of liquid to spawn.\n\nWARNING: no validation is done here." );
    if( ImGui::InputId( "liquid", liquid ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline( "Whether to spawn always, or with a chance." );
    if( ImGui::Checkbox( "Always", &spawn_always ) ) {
        state.mark_changed();
    }
    if( spawn_always ) {
        ImGui::BeginDisabled();
    }
    ImGui::HelpMarkerInline(
        "Chance to spawn, non-linear.\n\n"
        "Formula is:  one_in( rng( [min, max] ) )"
    );
    if( ImGui::InputIntRange( "chance", chance ) ) {
        state.mark_changed( "me-piece-liquid-chance-input" );
    }
    if( spawn_always ) {
        ImGui::EndDisabled();
    }
}

std::string PieceLiquid::fmt_data_summary() const
{
    return liquid.data;
}

void PieceIGroup::show_ui( State &state )
{
    ImGui::HelpMarkerInline(
        "Chance to spawn an item from the group, in percent.\n\n"
        "Accepted values: [1, 100].\n\n"
        "The value is multiplied by ITEM_SPAWNRATE option, and in case it overflows 100 - "
        "guarantees a spawn for every full 100%.\n\n"
        "For example: with chance of 70% and ITEM_SPAWNRATE of 4.1 the resulting chance would be "
        "287%, which results in 2 guaranteed spawns and 1 spawn with 87% chance."
    );
    if( ImGui::InputIntRange( "chance (%)", chance ) ) {
        state.mark_changed( "me-piece-igroup-chance-input" );
    }

    ImGui::HelpMarkerInline( "Whether to spawn once, or multiple times." );
    if( ImGui::Checkbox( "Once", &spawn_once ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( spawn_once );
    ImGui::HelpMarkerInline( "TODO" );
    if( ImGui::InputIntRange( "repeat", repeat ) ) {
        state.mark_changed( "me-piece-igroup-repeat-input" );
    }
    ImGui::EndDisabled();

    // TODO: inline item groups
    ImGui::HelpMarkerInline( "Item group to spawn." );
    if( ImGui::InputId( "group_id", group_id ) ) {
        state.mark_changed();
    }
}

std::string PieceIGroup::fmt_data_summary() const
{
    return group_id.data;
}

void PieceLoot::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceLoot::fmt_data_summary() const
{
    return "TODO";
}

void PieceMGroup::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Whether to spawn always, or with a chance." );
    if( ImGui::Checkbox( "Always", &spawn_always ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( spawn_always );
    ImGui::HelpMarkerInline( "TODO" );
    if( ImGui::InputIntRange( "chance", chance ) ) {
        state.mark_changed( "me-piece-mgroup-chance-input" );
    }
    ImGui::EndDisabled();

    ImGui::HelpMarkerInline( "Monster group to spawn." );
    if( ImGui::InputId( "group_id", group_id ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline( "Whether to use default map density, or a custom one." );
    if( ImGui::Checkbox( "Use default density", &use_default_density ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( use_default_density );
    ImGui::HelpMarkerInline( "TODO" );
    // TODO: validation
    if( ImGui::InputFloat( "density", &density ) ) {
        state.mark_changed( "me-piece-mgroup-density-input" );
    }
    ImGui::EndDisabled();
}

std::string PieceMGroup::fmt_data_summary() const
{
    return group_id.data;
}

void PieceMonster::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceMonster::fmt_data_summary() const
{
    return "TODO";
}

void PieceVehicle::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Vehicle group to spawn from." );
    if( ImGui::InputId( "group_id", group_id ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline( "Chance to spawn, in percent." );
    if( ImGui::InputIntRange( "chance (%)", chance ) ) {
        state.mark_changed( "me-piece-mgroup-chance-input" );
    }

    ImGui::HelpMarkerInline(
        "Vehicle status on spawn.\n\nTODO: details"
    );
    ImGui::Text( "Initial status:" );
    if( ImGui::RadioButton( "LightDamage", status == VehicleStatus::LightDamage ) ) {
        status = VehicleStatus::LightDamage;
    }
    if( ImGui::RadioButton( "Undamaged", status == VehicleStatus::Undamaged ) ) {
        status = VehicleStatus::Undamaged;
    }
    if( ImGui::RadioButton( "Disabled", status == VehicleStatus::Disabled ) ) {
        status = VehicleStatus::Disabled;
    }

    ImGui::HelpMarkerInline( "Whether to spawn with random percentage of fuel, or a set one." );
    if( ImGui::Checkbox( "Random fuel amount", &random_fuel_amount ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( random_fuel_amount );
    ImGui::HelpMarkerInline( "TODO" );
    if( ImGui::InputIntClamped( "fuel (%)", fuel, 0, 100 ) ) {
        state.mark_changed( "me-piece-vehicle-fuel-input" );
    }
    ImGui::EndDisabled();

    ImGui::Text( "Allowed rotations:" );
    if( ImGui::VehicleDirSet( allowed_rotations ) ) {
        state.mark_changed( "me-piece-vehicle-allowed-rotations-input" );
    }
}

std::string PieceVehicle::fmt_data_summary() const
{
    return group_id.data;
}

void PieceItem::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Item type to spawn." );
    if( ImGui::InputId( "item_id", item_id ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline( "Amount of items to spawn, [min, max]." );
    if( ImGui::InputIntRange( "amount", amount ) ) {
        state.mark_changed( "me-piece-item-amount-input" );
    }

    ImGui::HelpMarkerInline( "Whether to always spawn 1, or any amount with a chance." );
    if( ImGui::Checkbox( "Spawn one", &spawn_one ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( spawn_one );
    ImGui::HelpMarkerInline(
        "Chance to spawn item, in percent.\n\n"
        "Accepted values: [1, 99].\n\n"
        "The value is multiplied by ITEM_SPAWNRATE option, and in case it overflows 100 - "
        "guarantees a spawn for every full 100%.\n\n"
        "For example: with chance of 70% and ITEM_SPAWNRATE of 4.1 the resulting chance would be "
        "287%, which results in 2 guaranteed spawns and 1 spawn with 87% chance."
    );
    if( ImGui::InputIntRange( "chance (%)", chance ) ) {
        state.mark_changed( "me-piece-item-chance-input" );
    }
    ImGui::EndDisabled();

    ImGui::HelpMarkerInline( "Whether to spawn once, or multiple times." );
    if( ImGui::Checkbox( "Once", &spawn_once ) ) {
        state.mark_changed();
    }
    ImGui::BeginDisabled( spawn_once );
    ImGui::HelpMarkerInline( "TODO" );
    if( ImGui::InputIntRange( "repeat", repeat ) ) {
        state.mark_changed( "me-piece-item-repeat-input" );
    }
    ImGui::EndDisabled();
}

std::string PieceItem::fmt_data_summary() const
{
    return item_id.data;
}

void PieceTrap::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceTrap::fmt_data_summary() const
{
    return "TODO";
}

void PieceFurniture::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceFurniture::fmt_data_summary() const
{
    return "TODO";
}

void PieceTerrain::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceTerrain::fmt_data_summary() const
{
    return "TODO";
}

void PieceTerFurnTransform::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceTerFurnTransform::fmt_data_summary() const
{
    return "TODO";
}

void PieceMakeRubble::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceMakeRubble::fmt_data_summary() const
{
    return "TODO";
}

void PieceComputer::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceComputer::fmt_data_summary() const
{
    return "TODO";
}

void PieceSealeditem::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceSealeditem::fmt_data_summary() const
{
    return "TODO";
}

void PieceTranslate::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceTranslate::fmt_data_summary() const
{
    return "TODO";
}

void PieceZone::show_ui( State &state )
{
    ImGui::Text( "TODO" );
}

std::string PieceZone::fmt_data_summary() const
{
    return "TODO";
}

void PieceNested::show_ui( State &state )
{
    ImGui::Text( "TODO: more info" );

    show_weighted_list(state, list);
}

std::string PieceNested::fmt_data_summary() const
{
    std::string ret = list.entries[0].val.data;
    if (list.entries.size() > 1) {
        ret += string_format(" (+%d)", list.entries.size() - 1);
    }
    return ret;
}

void PieceNested::init_new()
{
    list.entries.emplace_back();
    list.entries.back().weight = 1;
}

std::unordered_set<point> PieceNested::silhouette() const
{
    std::unordered_set<point> ret;
    for (const auto& entry : list.entries) {
        if (entry.val.is_valid()) {
            const nested_mapgen& obj = entry.val.obj();
            for (const auto& it : obj.funcs()) {
                const mapgen_function_json_nested* func = it.obj.get();
                half_open_rectangle<point> rect(point_zero, func->mapgensize.raw());
                accumulate_points<point>(ret, rect);
            }
        }
    }
    return ret;
}

void PieceAltTrap::init_new()
{
    list.entries.emplace_back();
    list.entries.back().weight = 1;
}

void PieceAltTrap::show_ui( State &state )
{
    show_weighted_list( state, list );
}

std::string PieceAltTrap::fmt_data_summary() const
{
    std::string ret = list.entries[0].val.data;
    if( list.entries.size() > 1 ) {
        ret += string_format( " (+%d)", list.entries.size() - 1 );
    }
    return ret;
}

void PieceAltFurniture::init_new()
{
    list.entries.emplace_back();
    list.entries.back().weight = 1;
}

void PieceAltFurniture::show_ui( State &state )
{
    show_weighted_list( state, list );
}

std::string PieceAltFurniture::fmt_data_summary() const
{
    std::string ret = list.entries[0].val.data;
    if( list.entries.size() > 1 ) {
        ret += string_format( " (+%d)", list.entries.size() - 1 );
    }
    return ret;
}

void PieceAltTerrain::init_new()
{
    list.entries.emplace_back();
    list.entries.back().weight = 1;
}

void PieceAltTerrain::show_ui( State &state )
{
    show_weighted_list( state, list );
}

std::string PieceAltTerrain::fmt_data_summary() const
{
    std::string ret = list.entries[0].val.data;
    if( list.entries.size() > 1 ) {
        ret += string_format( " (+%d)", list.entries.size() - 1 );
    }
    return ret;
}

void PieceRemoveAll::show_ui(State& state)
{
    ImGui::Text("No data fields.");
    ImGui::Text("TODO: Fill in the description.");
}

std::string PieceRemoveAll::fmt_data_summary() const
{
    return "";
}

void PieceUnknown::show_ui(State& state)
{
    ImGui::Text("Unknown piece. Import failed.");
}

std::string PieceUnknown::fmt_data_summary() const
{
    return "unknown";
}

} // namespace editor
