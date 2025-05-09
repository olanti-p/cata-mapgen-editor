#include "piece_impl.h"

#include "common/algo.h"
#include "common/color.h"
#include "widget/style.h"
#include "state/state.h"
#include "widget/widgets.h"

// FIXME: header name conflict
#include "../../mapgen.h"
#include "vehicle_group.h"
#include "vehicle.h"
#include "item_factory.h"
#include "omdata.h"
#include "mongroup.h"

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

template<typename T>
void show_plain_list(State& state, std::vector<T>& list, std::function<void(size_t)> &&show_val)
{
    ImGui::Indent(style::list_indent);

    const auto can_delete = [&](size_t) -> bool {
        return true;
        };

    if (
        ImGui::VectorWidget()
        .with_for_each(std::move(show_val))
        .with_can_delete(can_delete)
        .with_default_add()
        .with_default_delete()
        .with_default_move()
        .with_default_drag_drop()
        .run(list)) {
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
    ImGui::HelpMarkerInline("Spawn chance, in percent.");
    if (ImGui::InputIntClamped("chance (%)", chance, 0, 100)) {
        state.mark_changed("me-piece-field-chance-input");
    }
    ImGui::HelpMarkerInline("Whether this will place field or remove field with matching id.");
    if (ImGui::Checkbox("remove", &remove)) {
        state.mark_changed();
    }

    ImGui::BeginDisabled(remove);
    ImGui::HelpMarkerInline( "Possible intensities of the field being placed." );
    ImGui::Text("intensity");
    ImGui::SameLine();
    ImVec2 sz(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
    ImGui::SetNextItemWidth(.3f );
    if (ImGui::Selectable("1", &int_1, ImGuiSelectableFlags_None, sz)) {
        state.mark_changed();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(.3f);
    if (ImGui::Selectable("2", &int_2, ImGuiSelectableFlags_None, sz)) {
        state.mark_changed();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(.3f);
    if (ImGui::Selectable("3", &int_3, ImGuiSelectableFlags_None, sz)) {
        state.mark_changed();
    }
    bool is_error = !remove && !int_1 && !int_2 && !int_3;
    if (is_error) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, col_error_text);
        ImGui::Text("Must select at least one");
        ImGui::PopStyleColor();
    }
    ImGui::HelpMarkerInline( "Age of the field at the moment of spawn.  Affects decay rate." );
    if( ImGui::InputDuration( "age", age ) ) {
        state.mark_changed();
    }
    ImGui::EndDisabled();
}

std::string PieceField::fmt_data_summary() const
{
    if (remove) {
        return string_format( "%s:remove", ftype.data );
    }
    else {
        return string_format( "%s:%s%s%s", ftype.data, int_1 ? "1" : "", int_2 ? "2" : "", int_3 ? "3" : "" );
    }
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
    ImGui::HelpMarkerInline("Unique id of this NPC (leave empty for nmo unique id).");
    if (ImGui::InputText("unique_id", &unique_id)) {
        state.mark_changed("me-piece-npc-unique-id-text");
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
    ImGui::HelpMarkerInline( "Faction id to apply." );
    if( ImGui::InputId( "id", id ) ) {
        state.mark_changed( "me-piece-faction-id-input" );
    }
}

std::string PieceFaction::fmt_data_summary() const
{
    return id.data;
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

    ImGui::HelpMarkerInline("Furniture tile to use. Must have SIGN flag.");
    if (ImGui::InputId("furniture", furniture)) {
        state.mark_changed();
    }
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
    ImGui::HelpMarkerInline( "Whether this vending machine is lootable." );
    if( ImGui::Checkbox( "lootable", &lootable ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Whether this vending machine is powered." );
    if( ImGui::Checkbox( "powered", &powered ) ) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline( "Whether this vending machine is networked." );
    if( ImGui::Checkbox( "networked", &networked ) ) {
        state.mark_changed();
    }
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

void PieceVendingMachine::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    item_group_id gid( use_default_group ? "default_vending_machine" : item_group.data);
    if (gid.is_valid()) {
        // TODO: ensure this is the correct way to spawn vending machine item group
        // TODO: time affects looted status
        const Item_spawn_data& group = *item_controller->get_group(gid);
        int num = roll_remainder(spawnrate);
        for (int i = 0; i < num; i++) {
            group.create(result, turn);
        }
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
    if (use_default_amount) {
        return "default";
    }
    else {
        return string_format( "[%d,%d]", amount.min, amount.max );
    }
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
    if( ImGui::RadioButton( "Random Diesel or Gasoline", fuel == GasPumpFuel::Random ) ) {
        fuel = GasPumpFuel::Random;
    }
    if( ImGui::RadioButton( "Diesel", fuel == GasPumpFuel::Diesel ) ) {
        fuel = GasPumpFuel::Diesel;
    }
    if( ImGui::RadioButton( "Gasoline", fuel == GasPumpFuel::Gasoline ) ) {
        fuel = GasPumpFuel::Gasoline;
    }
    if( ImGui::RadioButton( "JP8", fuel == GasPumpFuel::JP8 ) ) {
        fuel = GasPumpFuel::JP8;
    }
    if( ImGui::RadioButton( "AvGas", fuel == GasPumpFuel::AvGas ) ) {
        fuel = GasPumpFuel::AvGas;
    }
}

std::string PieceGaspump::fmt_data_summary() const
{
    switch (fuel) {
    case GasPumpFuel::Random:
        return "diesel/gasoline";
    case GasPumpFuel::Diesel:
        return "diesel";
    case GasPumpFuel::Gasoline:
        return "gasoline";
    case GasPumpFuel::JP8:
        return "jp8";
    case GasPumpFuel::AvGas:
        return "avgas";
    }
    // Shouldn't happen
    std::abort();
    return "";
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

void PieceLiquid::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    if (spawn_always || one_in(chance.roll())) {
        itype_id chosen_id(liquid.data);
        if (!chosen_id.is_valid()) {
            return;
        }

        // FIXME: this is copied from mapgen.cpp, undupe it
        itype_id migrated = item_controller->migrate_id(chosen_id);
        item newliquid(migrated, calendar::start_of_cataclysm);

        if (!use_default_amount && amount.min > -1) {
            if (amount.max > -1) {
                newliquid.charges = amount.roll();
            }
            else {
                newliquid.charges = amount.min;
            }

            if (migrated.str() == "gasoline" ||
                migrated.str() == "avgas" ||
                migrated.str() == "diesel" ||
                migrated.str() == "jp8") {
                newliquid.charges *= 100;
            }
        }
        if (newliquid.charges > 0) {
            result.push_back(newliquid);
        }
    }
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

	if (!is_object) {
		// IGroup having its own 'repeat' that duplicates into placement mode 'repeat' is so stupid...
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
	}

    // TODO: inline item groups
    ImGui::HelpMarkerInline( "Item group to spawn." );
    if( ImGui::InputId( "group_id", group_id ) ) {
        state.mark_changed();
    }

	ImGui::HelpMarkerInline("Owner faction. Leave as 'no_faction' to omit.");
    if (ImGui::InputId("faction", faction)) {
        state.mark_changed("me-piece-igroup-faction");
    }
}

std::string PieceIGroup::fmt_data_summary() const
{
    return group_id.data;
}

void PieceIGroup::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    item_group_id gid(group_id.data);
    if (gid.is_valid()) {
        const Item_spawn_data& group = *item_controller->get_group(gid);
        int num = roll_remainder(chance.roll() * spawnrate / 100.0f);
        for (int i = 0; i < num; i++) {
            group.create(result, turn);
        }
    }
}

void PieceLoot::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Spawn chance, in percent.");
    if (ImGui::InputIntClamped("chance (%)", chance, 0, 100)) {
        state.mark_changed("chance");
    }

    ImGui::HelpMarkerInline("Chance to spawn with ammo, in percent.");
    if (ImGui::InputIntClamped("ammo_chance (%)", ammo_chance, 0, 100)) {
        state.mark_changed("ammo_chance");
    }

    ImGui::HelpMarkerInline("Chance to spawn with magazine, in percent.");
    if (ImGui::InputIntClamped("magazine_chance (%)", magazine_chance, 0, 100)) {
        state.mark_changed("magazine_chance");
    }

    ImGui::HelpMarkerInline("Whether to spawn from item group.");
    if (ImGui::Checkbox("use item group", &is_group_mode)) {
        state.mark_changed();
    }

    ImGui::BeginDisabled(!is_group_mode);
    ImGui::HelpMarkerInline("Item group id.");
    if (ImGui::InputId("group", group_id)) {
        state.mark_changed("group_id");
    }
    ImGui::EndDisabled();

    ImGui::BeginDisabled(is_group_mode);
    ImGui::HelpMarkerInline("Item type id.");
    if (ImGui::InputId("item", item_id)) {
        state.mark_changed("item_id");
    }
    ImGui::HelpMarkerInline("Item variant, optional.");
    if (ImGui::InputText("variant", &variant)) {
        state.mark_changed("variant");
    }
    ImGui::EndDisabled();
}

std::string PieceLoot::fmt_data_summary() const
{
    if (is_group_mode) {
        return group_id.data;
    }
    else {
        return item_id.data;
    }
}

void PieceLoot::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    if (rng(0, 99) >= chance) {
        return;
    }
    Item_group result_group(Item_group::Type::G_COLLECTION, 100, ammo_chance, magazine_chance, "Map Editor Loot Designer");
    if (is_group_mode) {
        item_group_id id(group_id.data);
        if (!id.is_valid()) {
            return;
        }
        result_group.add_group_entry(id, 100);
    }
    else {
        itype_id id(item_id.data);
        if (!id.is_valid()) {
            return;
        }
        result_group.add_item_entry(item_controller->migrate_id(id), 100, variant);
    }
    Item_spawn_data::RecursionList rec;
    spawn_flags flags = spawn_flags::none;
    result_group.create(result, turn, rec, flags);
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

void PieceMonster::init_new()
{
    type_list.entries.emplace_back();
    type_list.entries.back().weight = 1;
}

void PieceMonster::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Whether to use monster group, or list of monster ids.");
    if (ImGui::Checkbox("Use group", &use_mongroup)) {
        state.mark_changed();
    }

    ImGui::BeginDisabled(!use_mongroup);
    ImGui::HelpMarkerInline("Monster group to spawn.");
    if (ImGui::InputId("group_id", group_id)) {
        state.mark_changed();
    }
    ImGui::EndDisabled();

    ImGui::BeginDisabled(use_mongroup);
    show_weighted_list(state, type_list);
    ImGui::EndDisabled();

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::InputIntRange("chance", chance)) {
        state.mark_changed("chance");
    }

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::Checkbox("use_pack_size", &use_pack_size)) {
        state.mark_changed("use_pack_size");
    }

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::InputIntRange("pack_size", pack_size)) {
        state.mark_changed("pack_size");
    }

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::ComboEnum("one_or_none", one_or_none)) {
        state.mark_changed("one_or_none");
    }

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::Checkbox("friendly", &friendly)) {
        state.mark_changed("friendly");
    }
    ImGui::SameLine();
    ImGui::HelpMarkerInline("TODO");
    if (ImGui::Checkbox("target", &target)) {
        state.mark_changed("target");
    }

    ImGui::HelpMarkerInline("TODO");
    if (ImGui::ComboEnum("name_mode", name_mode)) {
        state.mark_changed("name_mode");
    }

    ImGui::HelpMarkerInline("TODO");
    ImGui::BeginDisabled(name_mode != MonsterNameMode::Snippet && name_mode != MonsterNameMode::Exact);
    if (ImGui::InputText("name", &name)) {
        state.mark_changed("name");
    }
    ImGui::EndDisabled();

    // TODO: check for duplicates
    ImGui::Text("Ammo");
    ImGui::PushID("ammo");
    show_plain_list<std::pair<EID::Item, IntRange>>(state, ammo,
        [&](size_t i) {
            auto& it = ammo[i];
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
            if (ImGui::InputId("##ammo-type", it.first)) {
                state.mark_changed("list-ammo-type");
            }
            ImGui::HelpPopup("Ammo type.");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputIntRange("qty", it.second)) {
                state.mark_changed("list-entry-value");
            }
            ImGui::HelpPopup("Quantity.");
        }
    );
    ImGui::PopID();

    // TODO: visualization
    ImGui::Text("Patrol");
    ImGui::PushID("patrol");
    show_plain_list<point>(state, patrol,
        [&](size_t i) {
            auto& it = patrol[i];

            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 2.0f);
            if (ImGui::InputIntClamped("x", it.x, 0, 23)) {
                state.mark_changed("x");
            }
            ImGui::HelpPopup("Patrol point, in relative map squares.");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 2.0f);
            if (ImGui::InputIntClamped("y", it.y, 0, 23)) {
                state.mark_changed("y");
            }
            ImGui::HelpPopup("Patrol point, in relative map squares.");
        }
    );
    ImGui::PopID();
}

std::string PieceMonster::fmt_data_summary() const
{
    if (use_mongroup) {
        return group_id.data;
    }
    else {
        std::string ret = type_list.entries[0].val.data;
        if (type_list.entries.size() > 1) {
            ret += string_format(" (+%d)", type_list.entries.size() - 1);
        }
        return ret;
    }
}

void PieceVehicle::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Vehicle group to spawn from." );
    if( ImGui::InputId( "group_id", group_id ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline( "Chance to spawn, in percent." );
    if( ImGui::InputIntClamped( "chance (%)", chance, 0, 100 ) ) {
        state.mark_changed( "me-piece-vehicle-chance-input" );
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
    if( ImGui::RadioButton( "Pristine", status == VehicleStatus::Pristine ) ) {
        status = VehicleStatus::Pristine;
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

    ImGui::HelpMarkerInline("Owner faction. Leave as 'no_faction' to omit.");
    if (ImGui::InputId("faction", faction)) {
        state.mark_changed("me-piece-vehicle-faction");
    }
}

std::string PieceVehicle::fmt_data_summary() const
{
    return group_id.data;
}

void add_rotated_points( std::unordered_set<point>& points, vehicle& veh, units::angle dir) {
    veh.face.init(dir);
    veh.turn_dir = dir;
    veh.refresh();
    veh.precalc_mounts(0, veh.turn_dir, veh.pivot_anchor[0]);

    for (const vpart_reference& vp : veh.get_all_parts_with_fakes()) {
        tripoint_rel_ms p = vp.part().precalc[0];
        points.insert(p.raw().xy());
    }
}

std::unordered_set<point> PieceVehicle::silhouette() const
{
    std::unordered_set<point> ret;
    if (!group_id.is_valid()) {
        return ret;
    }

    // TODO: cache this
    const VehicleGroup& vg = group_id.obj();
    for (const vproto_id& proto_id : vg.all_possible_results()) {
        std::unique_ptr<vehicle> temp_veh = std::make_unique<vehicle>(proto_id);
        for (int rot : allowed_rotations) {
            add_rotated_points(ret, *temp_veh, units::from_degrees(rot));
        }
    }

    return ret;
}

void PieceItem::show_ui( State &state )
{
    ImGui::HelpMarkerInline( "Item type to spawn." );
    if( ImGui::InputId( "item_id", item_id ) ) {
        state.mark_changed();
    }

    ImGui::HelpMarkerInline("Item variant to spawn.");
    if (ImGui::InputText("variant", &variant)) {
        state.mark_changed("piece-item-variant");
    }

    ImGui::HelpMarkerInline( "Amount of items to spawn, [min, max]." );
    if( ImGui::InputIntRange( "amount", amount ) ) {
        state.mark_changed( "me-piece-item-amount-input" );
    }

    ImGui::HelpMarkerInline(
        "Chance to spawn item, in percent.\n\n"
        "Accepted values: [0, 100].\n\n"
        "The value is multiplied by ITEM_SPAWNRATE option, and in case it overflows 100 - "
        "guarantees a spawn for every full 100%.\n\n"
        "For example: with chance roll of 70% and ITEM_SPAWNRATE value of 4.1 the final chance would be "
        "287%, which results in 2 guaranteed spawns and 1 spawn with 87% chance.\n\n"
        "If chance is rolled as 100%, exactly 1 item will spawn regardless of ITEM_SPAWNRATE."
    );
    if( ImGui::InputIntRange( "chance (%)", chance ) ) {
        state.mark_changed( "me-piece-item-chance-input" );
    }

    if (!is_object) {
        // Pieces having its own 'repeat' that duplicates into placement mode 'repeat' is so stupid...
        ImGui::HelpMarkerInline("How many times to spawn.");
        if (ImGui::InputIntRange("repeat", repeat)) {
            state.mark_changed("me-piece-item-repeat-input");
        }
    }

    ImGui::HelpMarkerInline("Owner faction. Leave as 'no_faction' to omit.");
    if (ImGui::InputId("faction", faction)) {
        state.mark_changed("me-piece-item-faction");
    }

    // TODO: check for duplicates
    ImGui::SeparatorText("Custom Flags");
    ImGui::PushID("custom_flags");
    show_plain_list<EID::Flag>(state, custom_flags,
        [&](size_t i) {
            EID::Flag& it = custom_flags[i];

            if (ImGui::InputId("##flag", it)) {
                state.mark_changed("list-entry");
            }
            ImGui::HelpPopup("Custom flag to add.");
        }
    );
    ImGui::PopID();
}

std::string PieceItem::fmt_data_summary() const
{
    return item_id.data;
}

void PieceItem::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    itype_id chosen_id(item_id.data);
    if (chosen_id.is_valid()) {
        chosen_id = item_controller->migrate_id(chosen_id);

        const int c = chance.roll();
        int spawn_count = (c == 100) ? 1 : roll_remainder(c * spawnrate / 100.0f);

        const int quantity = amount.roll();

        // FIXME: copied from map.cpp, undupe it
        for (int i = 0; i < spawn_count * quantity; i++) {
            item new_item(chosen_id, calendar::start_of_cataclysm);
            new_item.set_itype_variant(variant);
            if (faction != default_faction) {
                new_item.set_owner(faction_id(faction.data));
            }
            if (one_in(3) && new_item.has_flag(flag_id("VARSIZE"))) {
                new_item.set_flag(flag_id("FIT"));
            }
            new_item = new_item.in_its_container();
            new_item.set_damage(0);
            new_item.rand_degradation();
            for (const auto& flag : custom_flags) {
                flag_id fid(flag.data);
                if (fid.is_valid()) {
                    new_item.set_flag(fid);
                }
            }

            result.push_back(new_item);
        }
    }

}

void PieceTrap::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Whether this will place specied trap or remove any trap.");
    if (ImGui::Checkbox("remove", &remove)) {
        state.mark_changed("piece-trap-remove-switch");
    }
    ImGui::BeginDisabled(remove);
    ImGui::HelpMarkerInline("Trap type to spawn.");
    if (ImGui::InputId("trap", id)) {
        state.mark_changed("piece-trap-id");
    }
    ImGui::EndDisabled();
}

std::string PieceTrap::fmt_data_summary() const
{
    if( remove ) {
        return "<remove>";
    } else {
        return id.data;
    }
}

void PieceFurniture::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Furniture type to spawn.");
    if (ImGui::InputId("furniture", id)) {
        state.mark_changed("piece-furn-id");
    }
}

std::string PieceFurniture::fmt_data_summary() const
{
    return id.data;
}

void PieceTerrain::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Terrain type to spawn.");
    if (ImGui::InputId("terrain", id)) {
        state.mark_changed("piece-ter-id");
    }
}

std::string PieceTerrain::fmt_data_summary() const
{
    return id.data;
}

void PieceTerFurnTransform::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Transform id to apply.");
    if (ImGui::InputId("transform", id)) {
        state.mark_changed("piece-ter-furn-transform-id");
    }
}

std::string PieceTerFurnTransform::fmt_data_summary() const
{
    return id.data;
}

void PieceMakeRubble::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Terrain id to use as floor.");
    if (ImGui::InputId("floor_type", floor_type)) {
        state.mark_changed("piece-rubble-floor-id");
    }
    ImGui::HelpMarkerInline("Furniture id to use for the pile.");
    if (ImGui::InputId("rubble_type", rubble_type)) {
        state.mark_changed("piece-rubble-rubble-type");
    }
    ImGui::HelpMarkerInline("Whether this will spawn items.");
    if (ImGui::Checkbox("items", &items)) {
        state.mark_changed();
    }
    ImGui::HelpMarkerInline("By default, rubble is generated from existing terrain and furniture.\nSetting it to overwrite will erase existing ter/furn before placing rubble.");
    if (ImGui::Checkbox("overwrite", &overwrite)) {
        state.mark_changed();
    }
}

std::string PieceMakeRubble::fmt_data_summary() const
{
    return rubble_type.data;
}

void PieceComputer::show_ui( State &state )
{
    static std::vector<std::string> failure_to_string;

    ImGui::HelpMarkerInline("In-game display name of the console.");
    if (ImGui::InputText("name", &name)) {
        state.mark_changed("piece-computer-name");
    }

    ImGui::HelpMarkerInline("'Access denied' message string.");
    if (ImGui::InputText("access_denied", &access_denied)) {
        state.mark_changed("piece-computer-access-denied");
    }

    ImGui::HelpMarkerInline("Login difficulty.");
    if (ImGui::InputIntClamped("security", security, 0, 100)) {
        state.mark_changed("piece-computer-login-security");
    }

    ImGui::HelpMarkerInline("Whether this computer is a mission target.");
    if (ImGui::Checkbox("mission target", &target)) {
        state.mark_changed();
    }

    ImGui::SeparatorText("Options");
    ImGui::PushID("options");
    show_plain_list<ComputerOption>(state, options,
        [&](size_t i) {
            ComputerOption& it = options[i];

            float x_pos = ImGui::GetCursorPosX();
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 15.0f);
            if (ImGui::InputText("##option-name", &it.name)) {
                state.mark_changed("option-name");
            }
            ImGui::HelpPopup("Option name.");

            ImGui::SetCursorPosX(x_pos);
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 15.0f);
            if (ImGui::ComboEnum("##option-action", it.action)) {
                state.mark_changed("option-action");
            }
            ImGui::HelpPopup("Action type.");

            ImGui::SetCursorPosX(x_pos);
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 15.0f);
            if (ImGui::InputIntClamped("##option-security", it.security, 0, 100)) {
                state.mark_changed("option-security");
            }
            ImGui::HelpPopup("Security level (difficulty).");
        }
    );
    ImGui::PopID();

    ImGui::SeparatorText("Failures");
    ImGui::PushID("failures");
    show_plain_list<ComputerFailure>(state, failures,
        [&](size_t i) {
            ComputerFailure& it = failures[i];

            if (ImGui::ComboEnum("##failure-type", it.action)) {
                state.mark_changed("list-entry");
            }
            ImGui::HelpPopup("Failure type.");
        }
    );
    ImGui::PopID();

    ImGui::SeparatorText("Chat Topics");
    ImGui::PushID("chat_topics");
    show_plain_list<std::string>(state, chat_topics,
        [&](size_t i) {
            std::string& it = chat_topics[i];

            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 15.0f);
            if (ImGui::InputText("##chat-topic", &it)) {
                state.mark_changed("list-entry");
            }
            // TODO: possible to validate this?
            ImGui::HelpPopup("Chat topic value (NO VALIDATION).");
        }
    );
    ImGui::PopID();

    ImGui::SeparatorText("EOCs");
    ImGui::PushID("eocs");
    show_plain_list<EID::EOC>(state, eocs,
        [&](size_t i) {
            EID::EOC& it = eocs[i];

            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 15.0f);
            if (ImGui::InputId("##eoc", it)) {
                state.mark_changed("list-entry");
            }
            ImGui::HelpPopup("EOC identifier");
        }
    );
    ImGui::PopID();
}

std::string PieceComputer::fmt_data_summary() const
{
    return name;
}

void PieceSealeditem::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Furniture id to spawn.");
    if (ImGui::InputId("furniture", furniture)) {
        state.mark_changed("furniture");
    }

    ImGui::HelpMarkerInline("Chance to spawn.");
    if (ImGui::InputIntRange("chance", chance)) {
        state.mark_changed("chance");
    }

    ImGui::SeparatorText("Item");
    ImGui::HelpMarkerInline("Whether to spawn item.");
    if (ImGui::Checkbox("use_item", &use_item)) {
        state.mark_changed("use_item");
    }

    ImGui::BeginDisabled(!use_item);
    ImGui::PushID("item");
    item_data.show_ui(state);
    ImGui::PopID();
    ImGui::EndDisabled();

    ImGui::SeparatorText("Group");
    ImGui::HelpMarkerInline("Whether to spawn item group.");
    if (ImGui::Checkbox("use_group", &use_group)) {
        state.mark_changed("use_group");
    }

    ImGui::BeginDisabled(!use_group);
    ImGui::PushID("group");
    group_data.show_ui(state);
    ImGui::PopID();
    ImGui::EndDisabled();
}

std::string PieceSealeditem::fmt_data_summary() const
{
    if (use_group) {
        return group_data.fmt_data_summary();
    }
    else if (use_item) {
        return item_data.fmt_data_summary();
    }
    else {
        return "<none>";
    }
}

void PieceSealeditem::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    // TODO: copied from source piece, undiplicate it
    const int c = chance.roll();
    const float spawn_rate = get_option<float>("ITEM_SPAWNRATE");
    if (!x_in_y((c == 100) ? 1 : c * spawn_rate / 100.0f, 1)) {
        return;
    }

    if (use_item) {
        item_data.roll_loot(result, turn, spawnrate);
    }
    if (use_group) {
        group_data.roll_loot(result, turn, spawnrate);
    }
}

void PieceZone::show_ui( State &state )
{
    ImGui::HelpMarkerInline("Zone type to place.");
    if (ImGui::InputId("zone_type", zone_type)) {
        state.mark_changed("piece-zone-zone-type");
    }
    ImGui::HelpMarkerInline("Faction id the zone belongs to.");
    if (ImGui::InputId("faction", faction)) {
        state.mark_changed("piece-zone-faction");
    }
    ImGui::HelpMarkerInline("In-game display name of the zone. Leave empty to omit.");
    if (ImGui::InputText("name", &name)) {
        state.mark_changed("piece-zone-name");
    }
    ImGui::HelpMarkerInline("Zone filter string, for LOOT_CUSTOM and LOOT_ITEM_GROUP zones. Leave empty to omit.");
    ImGui::BeginDisabled(zone_type.data != "LOOT_CUSTOM" && zone_type.data != "LOOT_ITEM_GROUP");
    if (ImGui::InputText("filter", &filter)) {
        state.mark_changed("piece-zone-filter");
    }
    ImGui::EndDisabled();
}

std::string PieceZone::fmt_data_summary() const
{
    if (!name.empty()) {
        return name;
    }
    else {
        return zone_type.data;
    }
}

std::unique_ptr<NestedCheck> NestedCheck::make(NestedCheckType type)
{
    switch (type) {
    case NestedCheckType::Oter: return std::make_unique<NestedCheckOter>();
    case NestedCheckType::Join: return std::make_unique<NestedCheckJoin>();
    case NestedCheckType::Flag: return std::make_unique<NestedCheckFlag>();
    case NestedCheckType::FlagAny: return std::make_unique<NestedCheckFlagAny>();
    case NestedCheckType::Predecessor: return std::make_unique<NestedCheckPredecessor>();
    case NestedCheckType::ZLevel: return std::make_unique<NestedCheckZLevel>();
    }
    std::abort();
    return nullptr;
}

NestedCheckOter::NestedCheckOter() {
    dir = direction::CENTER;
    matches.emplace_back();
    matches.back().first = "";
    matches.back().second = ot_match_type::contains;
}

void NestedCheckOter::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::ComboEnum("Match Direction", dir)) {
        state.mark_changed("dir");
    }

    // TODO: check for duplicates
    ImGui::Text("Match Values");
    ImGui::PushID("matches");
    show_plain_list<std::pair<std::string, ot_match_type>>(state, matches,
        [&](size_t i) {
            auto& it = matches[i];
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 5.0f);
            if (ImGui::ComboEnum("##entry-type", it.second)) {
                state.mark_changed("list-entry-type");
            }
            ImGui::HelpPopup("Match type.");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##entry-value", &it.first)) {
                state.mark_changed("list-entry-value");
            }
            ImGui::HelpPopup("Match string.");
        }
    );
    ImGui::PopID();
}

NestedCheckJoin::NestedCheckJoin() {
    dir = cube_direction::north;
    matches.emplace_back();
}

void NestedCheckJoin::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::ComboEnum("Match Direction", dir)) {
        state.mark_changed("dir");
    }

    // TODO: check for duplicates
    ImGui::Text("Match Values");
    ImGui::PushID("matches");
    show_plain_list<std::string>(state, matches,
        [&](size_t i) {
            auto& it = matches[i];
            if (ImGui::InputText("##entry", &it)) {
                state.mark_changed("list-entry");
            }
        }
    );
    ImGui::PopID();
}

NestedCheckFlag::NestedCheckFlag() {
    dir = direction::CENTER;
    matches.emplace_back(oter_flags::water);
}

void NestedCheckFlag::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::ComboEnum("Match Direction", dir)) {
        state.mark_changed("dir");
    }

    // TODO: check for duplicates
    ImGui::Text("Match Values");
    ImGui::PushID("matches");
    show_plain_list<oter_flags>(state, matches,
        [&](size_t i) {
            auto& it = matches[i];
            if (ImGui::ComboEnum("##entry", it)) {
                state.mark_changed("list-entry");
            }
        }
    );
    ImGui::PopID();
}

NestedCheckFlagAny::NestedCheckFlagAny() {
    dir = direction::CENTER;
    matches.emplace_back(oter_flags::water);
}

void NestedCheckFlagAny::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::ComboEnum("Match Direction", dir)) {
        state.mark_changed("dir");
    }

    // TODO: check for duplicates
    ImGui::Text("Match Values");
    ImGui::PushID("matches");
    show_plain_list<oter_flags>(state, matches,
        [&](size_t i) {
            auto& it = matches[i];
            if (ImGui::ComboEnum("##entry", it)) {
                state.mark_changed("list-entry");
            }
        }
    );
    ImGui::PopID();
}

NestedCheckPredecessor::NestedCheckPredecessor() {
    match_type = ot_match_type::contains;
}

void NestedCheckPredecessor::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::ComboEnum("##match_type", match_type)) {
        state.mark_changed("match_type");
    }
    ImGui::HelpPopup("Match type.");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputText("##terrain", &match_terrain)) {
        state.mark_changed("terrain");
    }
    ImGui::HelpPopup("Match string.");
}

NestedCheckZLevel::NestedCheckZLevel() {

}

void NestedCheckZLevel::show_ui(State& state) {
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 8.0f);
    if (ImGui::InputIntClamped("z", z, -10, 10)) {
        state.mark_changed("z");
    }
}

PieceNested::PieceNested(const PieceNested& rhs) : Piece(rhs) {
    chunks = rhs.chunks;
    else_chunks = rhs.else_chunks;
    preview = rhs.preview;
    preview_pos = rhs.preview_pos;
    checks.clear();
    for (const auto& it : rhs.checks) {
        checks.emplace_back(it->clone());
    }
}

static void add_nested_chunk(std::vector<std::string>& opts, EID::Nest id) {
    const auto& base_id = id.data;
    auto it = editor_mapgen_nested_options.find(base_id);
    if (it != editor_mapgen_nested_options.end()) {
        for (const auto& entry : it->second) {
            opts.push_back(entry);
        }
    }
}

std::vector<std::string> PieceNested::build_preview_options() const
{
    // TODO: persist this
    std::vector<std::string> preview_opts;
    preview_opts.emplace_back("");
    for (const auto& it : chunks.entries) {
        add_nested_chunk(preview_opts, it.val);
    }
    for (const auto& it : else_chunks.entries) {
        add_nested_chunk(preview_opts, it.val);
    }
    return preview_opts;
}

void PieceNested::show_ui(State& state)
{
    {
        std::vector<std::string> preview_opts = build_preview_options();
        auto it = std::find(preview_opts.begin(), preview_opts.end(), preview);
        int current_idx = it == preview_opts.end() ? -1 : static_cast<int>(std::distance(preview_opts.begin(), it));
        if (ImGui::ComboWithFilter("preview", &current_idx, preview_opts, 10)) {
            preview = preview_opts[current_idx];
            state.mark_changed();
        }
    }

    if (is_object) {
        if (ImGui::InputPoint("preview pos", preview_pos)) {
            state.mark_changed("preview_pos");
        }
    }

    ImGui::PushID("chunks");
    ImGui::SeparatorText("Chunks");
    show_weighted_list(state, chunks);
    ImGui::PopID();

    ImGui::PushID("else-chunks");
    ImGui::SeparatorText("Else Chunks");
    show_weighted_list(state, else_chunks);
    ImGui::PopID();

    ImGui::PushID("checks");
    ImGui::SeparatorText("Checks");
    ImGui::Indent(style::list_indent);

    const auto show_val = [&](size_t i) {
        NestedCheck& entry = *checks[i].get();

        float indent = ImGui::GetFrameHeight() * 2.3f;
        ImGui::Text("%s", io::enum_to_string<NestedCheckType>(entry.get_type()));
        ImGui::Indent(indent);
        entry.show_ui(state);
        ImGui::Indent(-indent);
    };

    const auto show_add = [&]() -> bool {
        bool ret = false;

        if (ImGui::Button("+ Oter")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::Oter));
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Flag")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::Flag));
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Flag Any")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::FlagAny));
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Join")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::Join));
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Z Level")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::ZLevel));
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Predecessor")) {
            checks.emplace_back(NestedCheck::make(NestedCheckType::Predecessor));
        }

        return ret;
    };

    if (
        ImGui::VectorWidget()
        .with_for_each(show_val)
        .with_default_delete()
        .with_default_move()
        .with_default_drag_drop()
        .with_add(show_add)
        .run(checks)) {
        state.mark_changed();
    }

    ImGui::Indent(-style::list_indent);
    ImGui::PopID();
}

std::string PieceNested::fmt_data_summary() const
{
    std::string ret;
    if (chunks.entries.size() > 0) {
        ret += chunks.entries[0].val.data;
        if (chunks.entries.size() > 1) {
            ret += string_format(" (+%d)", chunks.entries.size() - 1 + else_chunks.entries.size() - 1);
        }
    }
    else if (else_chunks.entries.size() > 0) {
        ret += "ELSE ";
        ret += else_chunks.entries[0].val.data;
        if (else_chunks.entries.size() > 1) {
            ret += string_format(" (+%d)", else_chunks.entries.size() - 1);
        }
    }
    else {
        ret += "<null>";
    }
    return ret;
}

void PieceNested::init_new()
{
    chunks.entries.emplace_back();
    chunks.entries.back().weight = 1;
}

std::unordered_set<point> PieceNested::silhouette() const
{
    std::unordered_set<point> ret;
    for (const auto& entry : chunks.entries) {
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

void PieceRemoveVehicles::show_ui(State& state)
{
    show_plain_list<EID::Vehicle>(state, list, [&](size_t idx) {
        auto& it = list[idx];
        if (ImGui::InputId("##veh-type", it)) {
            state.mark_changed("list-veh-type");
        }
        ImGui::HelpPopup("Vehicle type to remove.\n\nLeave empty to skip type check.");
    });
}

std::string PieceRemoveVehicles::fmt_data_summary() const
{
    if (list.empty()) {
        return "<any>";
    }
    else {
        std::string ret = list[0].data;
        if (list.size() > 1) {
            ret += string_format(" (+%d)", list.size() - 1);
        }
        return ret;
    }
}

void PieceRemoveNPCs::show_ui(State& state)
{
    if (ImGui::InputText("npc_class", &npc_class)) {
        state.mark_changed("npc-class");
    }
    ImGui::HelpPopup("NPC must match this class.\n\nLeave empty to skip class check.");

    if (ImGui::InputText("unique_id", &unique_id)) {
        state.mark_changed("unique-id");
    }
    ImGui::HelpPopup("NPC must have this unique id.\n\nLeave empty to skip unique id check.");
}

std::string PieceRemoveNPCs::fmt_data_summary() const
{
    if (npc_class.empty() && unique_id.empty()) {
        return "<any>";
    }
    else if (!unique_id.empty()) {
        return unique_id;
    }
    else {
        return npc_class;
    }
}

void PieceCorpse::show_ui(State& state)
{
    if (ImGui::InputId("group", group)) {
        state.mark_changed("group");
    }
    if (ImGui::InputIntClamped("age (days)", age_days, 0, INT_MAX)) {
        state.mark_changed("age-days");
    }
}

std::string PieceCorpse::fmt_data_summary() const
{
    return group.data;
}

void PieceCorpse::roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const
{
    // TODO: undupe with mapgen.cpp
    mongroup_id group_id(group.data);
    if (group_id.is_valid()) {
        const std::vector<mtype_id> monster_group =
            MonsterGroupManager::GetMonstersFromGroup(group_id, true);
        const mtype_id& corpse_type = random_entry_ref(monster_group);
        item corpse = item::make_corpse(corpse_type,
            std::max(calendar::turn - time_duration::from_days( age_days ), calendar::start_of_cataclysm));
        result.push_back(corpse);
    }
}

void PieceVariable::show_ui(State& state)
{
    if (ImGui::InputText("name", &name)) {
        state.mark_changed("name");
    }
}

std::string PieceVariable::fmt_data_summary() const
{
    return name;
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
