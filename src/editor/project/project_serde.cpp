#include "project_serde.h"

#include "common/canvas_2d_serde.h"
#include "common/weighted_list_serde.h"
#include "common/map_key.h"
#include <imgui/imgui.h>
#include "json.h"
#include "mapgen.h"
#include "mapgen/mapgen.h"
#include "mapgen/piece_impl.h"
#include "mapgen/piece_type.h"
#include "mapgen/selection_mask.h"
#include "mapgen/setmap.h"
#include "mapgen/setmap_impl.h"
#include "project.h"

#include "cube_direction.h"
#include "omdata.h"

void serialize( const std::unique_ptr<editor::Piece> &ptr, JsonOut &jsout )
{
    jsout.start_object();
    jsout.member_as_string( "piece_type", ptr->get_type() );
    jsout.member( "uuid", ptr->uuid );
    jsout.member("constraint", ptr->constraint);
    ptr->serialize( jsout );
    jsout.end_object();
}

void deserialize( std::unique_ptr<editor::Piece> &ptr, const TextJsonObject &jo )
{
    editor::PieceType pt = editor::PieceType::NumJmTypes;
    jo.read( "piece_type", pt );
    editor::UUID uuid;
    jo.read( "uuid", uuid );
    std::optional<editor::PieceConstraint> constraint;
    jo.read("constraint", constraint);

    std::unique_ptr<editor::Piece> val = editor::make_new_piece( pt );
    val->deserialize( jo );
    val->uuid = uuid;
    val->constraint = std::move( constraint );
    ptr = std::move( val );
}

void serialize(const std::unique_ptr<editor::SetMapData>& ptr, JsonOut& jsout)
{
    jsout.start_object();
    jsout.member_as_string("data_type", ptr->get_type());
    ptr->serialize(jsout);
    jsout.end_object();
}

void deserialize(std::unique_ptr<editor::SetMapData>& ptr, const TextJsonObject& jo)
{
    editor::SetMapType pt = editor::SetMapType::_Num;
    jo.read("data_type", pt);

    std::unique_ptr<editor::SetMapData> val = editor::make_new_setmap_data(pt);
    val->deserialize(jo);
    ptr = std::move(val);
}

void serialize( const ImVec4 &v, JsonOut &jsout )
{
    jsout.start_object();
    jsout.member( "x", v.x );
    jsout.member( "y", v.y );
    jsout.member( "z", v.z );
    jsout.member( "w", v.w );
    jsout.end_object();
}

void deserialize( ImVec4 &v, const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "x", v.x );
    jo.read( "y", v.y );
    jo.read( "z", v.z );
    jo.read( "w", v.w );
}

namespace io
{

template<>
std::string enum_to_string<editor::OterMapgenFill>( editor::OterMapgenFill data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::OterMapgenFill::None: return "None";
        case editor::OterMapgenFill::FillTer: return "FillTer";
        case editor::OterMapgenFill::PredecessorMapgen: return "PredecessorMapgen";
        case editor::OterMapgenFill::FallbackPredecessorMapgen: return "FallbackPredecessorMapgen";
        // *INDENT-ON*
        case editor::OterMapgenFill::_Num:
            break;
    }
    debugmsg( "Invalid editor::OterMapgenFill" );
    abort();
}

template<>
std::string enum_to_string<editor::LayeringRuleTrapFurn>(editor::LayeringRuleTrapFurn data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::LayeringRuleTrapFurn::Unspecified: return "Unspecified";
    case editor::LayeringRuleTrapFurn::Allow: return "Allow";
    case editor::LayeringRuleTrapFurn::Dismantle: return "Dismantle";
    case editor::LayeringRuleTrapFurn::Erase: return "Erase";
        // *INDENT-ON*
    case editor::LayeringRuleTrapFurn::_Num:
        break;
    }
    debugmsg("Invalid editor::LayeringRuleTrapFurn");
    abort();
}

template<>
std::string enum_to_string<editor::LayeringRuleItems>(editor::LayeringRuleItems data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::LayeringRuleItems::Unspecified: return "Unspecified";
    case editor::LayeringRuleItems::Allow: return "Allow";
    case editor::LayeringRuleItems::Erase: return "Erase";
        // *INDENT-ON*
    case editor::LayeringRuleItems::_Num:
        break;
    }
    debugmsg("Invalid editor::LayeringRuleItems");
    abort();
}

template<>
std::string enum_to_string<editor::MapgenType>( editor::MapgenType data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::MapgenType::Nested: return "Nested";
        case editor::MapgenType::Oter: return "Oter";
        case editor::MapgenType::Update: return "Update";
        // *INDENT-ON*
        case editor::MapgenType::_Num:
            break;
    }
    debugmsg( "Invalid editor::MapgenType" );
    abort();
}

template<>
std::string enum_to_string<editor::GasPumpFuel>( editor::GasPumpFuel data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::GasPumpFuel::Random: return "Random";
        case editor::GasPumpFuel::Diesel: return "Diesel";
        case editor::GasPumpFuel::Gasoline: return "Gasoline";
        case editor::GasPumpFuel::JP8: return "JP8";
        case editor::GasPumpFuel::AvGas: return "AvGas";
        // *INDENT-ON*
        case editor::GasPumpFuel::_Num:
            break;
    }
    debugmsg( "Invalid editor::GasPumpFuel" );
    abort();
}

template<>
std::string enum_to_string<editor::VehicleStatus>( editor::VehicleStatus data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::VehicleStatus::LightDamage: return "LightDamage";
        case editor::VehicleStatus::Undamaged: return "Undamaged";
        case editor::VehicleStatus::Disabled: return "Disabled";
        case editor::VehicleStatus::Pristine: return "Pristine";
        // *INDENT-ON*
        case editor::VehicleStatus::_Num:
            break;
    }
    debugmsg( "Invalid editor::VehicleStatus" );
    abort();
}

template<>
std::string enum_to_string<editor::NestedCheckType>(editor::NestedCheckType data)
{
    switch (data) {
        // *INDENT-OFF*
        case editor::NestedCheckType::Oter: return "Oter";
        case editor::NestedCheckType::Join: return "Join";
        case editor::NestedCheckType::Flag: return "Flag";
        case editor::NestedCheckType::FlagAny: return "FlagAny";
        case editor::NestedCheckType::Predecessor: return "Predecessor";
        case editor::NestedCheckType::ZLevel: return "ZLevel";
        // *INDENT-ON*
        case editor::NestedCheckType::_Num:
            break;
    }
    debugmsg("Invalid editor::NestedCheckType");
    abort();
}

template<>
std::string enum_to_string<editor::PieceType>( editor::PieceType data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::PieceType::Field: return "Field";
        case editor::PieceType::NPC: return "NPC";
        case editor::PieceType::Faction: return "Faction";
        case editor::PieceType::Sign: return "Sign";
        case editor::PieceType::Graffiti: return "Graffiti";
        case editor::PieceType::VendingMachine: return "VendingMachine";
        case editor::PieceType::Toilet: return "Toilet";
        case editor::PieceType::GasPump: return "GasPump";
        case editor::PieceType::Liquid: return "Liquid";
        case editor::PieceType::IGroup: return "Igroup";
        case editor::PieceType::Loot: return "Loot";
        case editor::PieceType::MGroup: return "Mgroup";
        case editor::PieceType::Monster: return "Monster";
        case editor::PieceType::Vehicle: return "Vehicle";
        case editor::PieceType::Item: return "Item";
        case editor::PieceType::Trap: return "Trap";
        case editor::PieceType::Furniture: return "Furniture";
        case editor::PieceType::Terrain: return "Terrain";
        case editor::PieceType::TerFurnTransform: return "TerFurnTransform";
        case editor::PieceType::MakeRubble: return "MakeRubble";
        case editor::PieceType::Computer: return "Computer";
        case editor::PieceType::SealedItem: return "SealedItem";
        case editor::PieceType::Zone: return "Zone";
        case editor::PieceType::Nested: return "Nested";
        case editor::PieceType::AltTrap: return "AltTrap";
        case editor::PieceType::AltFurniture: return "AltFurniture";
        case editor::PieceType::AltTerrain: return "AltTerrain";
        case editor::PieceType::RemoveAll: return "RemoveAll";
        case editor::PieceType::Unknown: return "Unknown";
        // *INDENT-ON*
        default:
            break;
    }
    debugmsg( "Invalid editor::PieceType" );
    abort();
}

template<>
std::string enum_to_string<editor::SetMapMode>(editor::SetMapMode data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::SetMapMode::Point: return "Point";
    case editor::SetMapMode::Line: return "Line";
    case editor::SetMapMode::Square: return "Square";
        // *INDENT-ON*
    case editor::SetMapMode::_Num:
        break;
    }
    debugmsg("Invalid editor::SetMapMode");
    abort();
}

template<>
std::string enum_to_string<editor::SetMapType>(editor::SetMapType data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::SetMapType::Ter: return "Ter";
    case editor::SetMapType::Furn: return "Furn";
    case editor::SetMapType::Trap: return "Trap";
    case editor::SetMapType::Variable: return "Variable";
    case editor::SetMapType::Bash: return "Bash";
    case editor::SetMapType::Burn: return "Burn";
    case editor::SetMapType::Radiation: return "Radiation";
    case editor::SetMapType::RemoveTrap: return "RemoveTrap";
    case editor::SetMapType::RemoveCreature: return "RemoveCreature";
    case editor::SetMapType::RemoveItem: return "RemoveItem";
    case editor::SetMapType::RemoveField: return "RemoveField";
        // *INDENT-ON*
    case editor::SetMapType::_Num:
        break;
    }
    debugmsg("Invalid editor::SetMapType");
    abort();
}

template<>
std::string enum_to_string<editor::MonsterNameMode>(editor::MonsterNameMode data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::MonsterNameMode::Unnamed: return "Unnamed";
    case editor::MonsterNameMode::Exact: return "Exact";
    case editor::MonsterNameMode::Random: return "Random";
    case editor::MonsterNameMode::Male: return "Male";
    case editor::MonsterNameMode::Female: return "Female";
    case editor::MonsterNameMode::Snippet: return "Snippet";
        // *INDENT-ON*
    case editor::MonsterNameMode::_Num:
        break;
    }
    debugmsg("Invalid editor::MonsterNameMode");
    abort();
}

template<>
std::string enum_to_string<editor::OneOrNoneMode>(editor::OneOrNoneMode data)
{
    switch (data) {
        // *INDENT-OFF*
    case editor::OneOrNoneMode::On: return "On";
    case editor::OneOrNoneMode::Off: return "Off";
    case editor::OneOrNoneMode::Default: return "Default";
        // *INDENT-ON*
    case editor::OneOrNoneMode::_Num:
        break;
    }
    debugmsg("Invalid editor::OneOrNoneMode");
    abort();
}

} // namespace io

namespace editor
{
void PieceConstraint::deserialize(const JSON_OBJECT& jsin)
{
    // TODO
}

void PieceConstraint::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    // TODO
    jsout.end_object();
}

void PieceField::serialize( JsonOut &jsout ) const
{
    jsout.member( "ftype", ftype );
    jsout.member( "chance", chance );
    jsout.member( "remove", remove );
    jsout.member( "int_1", int_1 );
    jsout.member( "int_2", int_2 );
    jsout.member( "int_3", int_3 );
    jsout.member( "age", age );
}

void PieceField::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "ftype", ftype );
    jsin.read( "chance", chance );
    jsin.read( "remove", remove );
    jsin.read( "int_1", int_1 );
    jsin.read( "int_2", int_2 );
    jsin.read( "int_3", int_3 );
    jsin.read( "age", age );
}

void PieceNPC::serialize( JsonOut &jsout ) const
{
    jsout.member( "npc_class", npc_class );
    jsout.member( "target", target );
    jsout.member( "traits", traits );
    jsout.member( "unique_id", unique_id );
}

void PieceNPC::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "npc_class", npc_class );
    jsin.read( "target", target );
    jsin.read( "traits", traits );
    jsin.read( "unique_id", unique_id );
}

void PieceFaction::serialize( JsonOut &jsout ) const
{
    jsout.member( "id", id );
}

void PieceFaction::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "id", id );
}

void PieceSign::serialize( JsonOut &jsout ) const
{
    jsout.member( "use_snippet", use_snippet );
    jsout.member( "snippet", snippet );
    jsout.member( "text", text );
    jsout.member( "furniture", furniture);
}

void PieceSign::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_snippet", use_snippet );
    jsin.read( "snippet", snippet );
    jsin.read( "text", text );
    jsin.read( "furniture", furniture);
}

void PieceGraffiti::serialize( JsonOut &jsout ) const
{
    jsout.member( "use_snippet", use_snippet );
    jsout.member( "snippet", snippet );
    jsout.member( "text", text );
}

void PieceGraffiti::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_snippet", use_snippet );
    jsin.read( "snippet", snippet );
    jsin.read( "text", text );
}

void PieceVendingMachine::serialize( JsonOut &jsout ) const
{
    jsout.member( "lootable", lootable );
    jsout.member( "powered", powered );
    jsout.member( "networked", networked );
    jsout.member( "reinforced", reinforced );
    jsout.member( "use_default_group", use_default_group );
    jsout.member( "item_group", item_group );
}

void PieceVendingMachine::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "lootable", lootable );
    jsin.read( "powered", powered );
    jsin.read( "networked", networked );
    jsin.read( "reinforced", reinforced );
    jsin.read( "use_default_group", use_default_group );
    jsin.read( "item_group", item_group );
}

void PieceToilet::serialize( JsonOut &jsout ) const
{
    jsout.member( "use_default_amount", use_default_amount );
    jsout.member( "amount", amount );
}

void PieceToilet::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_default_amount", use_default_amount );
    jsin.read( "amount", amount );
}

void PieceGaspump::serialize( JsonOut &jsout ) const
{
    jsout.member( "use_default_amount", use_default_amount );
    jsout.member( "amount", amount );
    jsout.member_as_string( "fuel", fuel );
}

void PieceGaspump::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_default_amount", use_default_amount );
    jsin.read( "amount", amount );
    jsin.read( "fuel", fuel );
}

void PieceLiquid::serialize( JsonOut &jsout ) const
{
    jsout.member( "use_default_amount", use_default_amount );
    jsout.member( "amount", amount );
    jsout.member( "liquid", liquid );
    jsout.member( "spawn_always", spawn_always );
    jsout.member( "chance", chance );
}

void PieceLiquid::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_default_amount", use_default_amount );
    jsin.read( "amount", amount );
    jsin.read( "liquid", liquid );
    jsin.read( "spawn_always", spawn_always );
    jsin.read( "chance", chance );
}

void PieceIGroup::serialize( JsonOut &jsout ) const
{
    jsout.member( "chance", chance );
    jsout.member( "group_id", group_id );
    jsout.member( "spawn_once", spawn_once );
    jsout.member( "repeat", repeat );
    jsout.member( "faction", faction );
}

void PieceIGroup::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "chance", chance );
    jsin.read( "group_id", group_id );
    jsin.read( "spawn_once", spawn_once );
    jsin.read( "repeat", repeat );
    jsin.read( "faction", faction );
}

void PieceLoot::serialize( JsonOut &jsout ) const
{
    jsout.member("chance", chance);
    jsout.member("ammo_chance", ammo_chance);
    jsout.member("magazine_chance", magazine_chance);
    jsout.member("is_group_mode", is_group_mode);
    jsout.member("group_id", group_id);
    jsout.member("item_id", item_id);
    jsout.member("variant", variant);
}

void PieceLoot::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("chance", chance);
    jsin.read("ammo_chance", ammo_chance);
    jsin.read("magazine_chance", magazine_chance);
    jsin.read("is_group_mode", is_group_mode);
    jsin.read("group_id", group_id);
    jsin.read("item_id", item_id);
    jsin.read("variant", variant);
}

void PieceMGroup::serialize( JsonOut &jsout ) const
{
    jsout.member( "spawn_always", spawn_always );
    jsout.member( "chance", chance );
    jsout.member( "group_id", group_id );
    jsout.member( "use_default_density", use_default_density );
    jsout.member( "density", density );
}

void PieceMGroup::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "spawn_always", spawn_always );
    jsin.read( "chance", chance );
    jsin.read( "group_id", group_id );
    jsin.read( "use_default_density", use_default_density );
    jsin.read( "density", density );
}

void PieceMonster::serialize( JsonOut &jsout ) const
{
    jsout.member("use_mongroup", use_mongroup);
    jsout.member("type_list", type_list);
    jsout.member("group_id", group_id);
    jsout.member("chance", chance);
    jsout.member("use_pack_size", use_pack_size);
    jsout.member("pack_size", pack_size);
    jsout.member_as_string("one_or_none", one_or_none);
    jsout.member("friendly", friendly);
    jsout.member("target", target);
    jsout.member_as_string("name_mode", name_mode);
    jsout.member("name", name);
    jsout.member("ammo", ammo);
    jsout.member("patrol", patrol);
}

void PieceMonster::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("use_mongroup", use_mongroup);
    jsin.read("type_list", type_list);
    jsin.read("group_id", group_id);
    jsin.read("chance", chance);
    jsin.read("use_pack_size", use_pack_size);
    jsin.read("pack_size", pack_size);
    jsin.read("one_or_none", one_or_none);
    jsin.read("friendly", friendly);
    jsin.read("target", target);
    jsin.read("name_mode", name_mode);
    jsin.read("name", name);
    jsin.read("ammo", ammo);
    jsin.read("patrol", patrol);
}

void PieceVehicle::serialize( JsonOut &jsout ) const
{
    jsout.member( "group_id", group_id );
    jsout.member( "chance", chance );
    jsout.member_as_string( "status", status );
    jsout.member( "random_fuel_amount", random_fuel_amount );
    jsout.member( "fuel", fuel );
    jsout.member( "allowed_rotations", allowed_rotations );
    jsout.member( "faction", faction );
}

void PieceVehicle::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "group_id", group_id );
    jsin.read( "chance", chance );
    jsin.read( "status", status );
    jsin.read( "random_fuel_amount", random_fuel_amount );
    jsin.read( "fuel", fuel );
    jsin.read( "allowed_rotations", allowed_rotations );
    jsin.read( "faction", faction );
}

void PieceItem::serialize( JsonOut &jsout ) const
{
    jsout.member( "item_id", item_id );
    jsout.member( "variant", variant );
    jsout.member( "amount", amount );
    jsout.member( "chance", chance );
    jsout.member( "repeat", repeat );
    jsout.member( "custom_flags", custom_flags );
    jsout.member( "faction", faction );
}

void PieceItem::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "item_id", item_id );
    jsin.read( "variant", variant );
    jsin.read( "amount", amount );
    jsin.read( "chance", chance );
    jsin.read( "repeat", repeat );
    jsin.read( "custom_flags", custom_flags );
    jsin.read( "faction", faction );
}

void PieceTrap::serialize( JsonOut &jsout ) const
{
    jsout.member("id", id);
    jsout.member("remove", remove);
}

void PieceTrap::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("id", id);
    jsin.read("remove", remove);
}

void PieceFurniture::serialize( JsonOut &jsout ) const
{
    jsout.member("id", id);
}

void PieceFurniture::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("id", id);
}

void PieceTerrain::serialize( JsonOut &jsout ) const
{
    jsout.member("id", id);
}

void PieceTerrain::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("id", id);
}

void PieceTerFurnTransform::serialize( JsonOut &jsout ) const
{
    jsout.member("id", id);
}

void PieceTerFurnTransform::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("id", id);
}

void PieceMakeRubble::serialize( JsonOut &jsout ) const
{
    jsout.member("items", items);
    jsout.member("overwrite", overwrite);
    jsout.member("rubble_type", rubble_type);
    jsout.member("floor_type", floor_type);
}

void PieceMakeRubble::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("items", items);
    jsin.read("overwrite", overwrite);
    jsin.read("rubble_type", rubble_type);
    jsin.read("floor_type", floor_type);
}

void ComputerOption::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member("name", name);
    jsout.member_as_string("action", action);
    jsout.member("security", security);
    jsout.end_object();
}

void ComputerOption::deserialize(const TextJsonObject& jsin)
{
    jsin.read("name", name);
    jsin.read("action", action);
    jsin.read("security", security);
}

void ComputerFailure::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member_as_string("action", action);
    jsout.end_object();
}

void ComputerFailure::deserialize(const TextJsonObject& jsin)
{
    jsin.read("action", action);
}

void PieceComputer::serialize( JsonOut &jsout ) const
{
    jsout.member("name", name);
    jsout.member("access_denied", access_denied);
    jsout.member("security", security);
    jsout.member("target", target);
    jsout.member("options", options);
    jsout.member("failures", failures);
    jsout.member("chat_topics", chat_topics);
    jsout.member("eocs", eocs);
}

void PieceComputer::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("name", name);
    jsin.read("access_denied", access_denied);
    jsin.read("security", security);
    jsin.read("target", target);
    jsin.read("options", options);
    jsin.read("failures", failures);
    jsin.read("chat_topics", chat_topics);
    jsin.read("eocs", eocs);
}

void PieceSealeditem::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceSealeditem::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceZone::serialize( JsonOut &jsout ) const
{
    jsout.member("zone_type", zone_type);
    jsout.member("faction", faction);
    jsout.member("name", name);
    jsout.member("filter", filter);
}

void PieceZone::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("zone_type", zone_type);
    jsin.read("faction", faction);
    jsin.read("name", name);
    jsin.read("filter", filter);
}

void NestedCheckOter::serialize(JsonOut& jsout) const
{
    jsout.member_as_string("dir", dir);
    jsout.member("matches", matches);
}

void NestedCheckOter::deserialize(const JSON_OBJECT& jsin)
{
    dir = jsin.get_enum_value<direction>("dir");
    jsin.read("matches", matches);
}

void NestedCheckJoin::serialize(JsonOut& jsout) const
{
    jsout.member_as_string("dir", dir);
    jsout.member("matches", matches);
}

void NestedCheckJoin::deserialize(const JSON_OBJECT& jsin)
{
    dir = jsin.get_enum_value<cube_direction>("dir");
    jsin.read("matches", matches);
}

void NestedCheckFlag::serialize(JsonOut& jsout) const
{
    jsout.member_as_string("dir", dir);
    jsout.member("matches", matches);
}

void NestedCheckFlag::deserialize(const JSON_OBJECT& jsin)
{
    dir = jsin.get_enum_value<direction>("dir");
    jsin.read("matches", matches);
}

void NestedCheckFlagAny::serialize(JsonOut& jsout) const
{
    jsout.member_as_string("dir", dir);
    jsout.member("matches", matches);
}

void NestedCheckFlagAny::deserialize(const JSON_OBJECT& jsin)
{
    dir = jsin.get_enum_value<direction>("dir");
    jsin.read("matches", matches);
}

void NestedCheckPredecessor::serialize(JsonOut& jsout) const
{
    jsout.member_as_string("match_type", match_type);
    jsout.member("match_terrain", match_terrain);
}

void NestedCheckPredecessor::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("match_type", match_type);
    jsin.read("match_terrain", match_terrain);
}

void NestedCheckZLevel::serialize(JsonOut& jsout) const
{
    jsout.member("z", z);
}

void NestedCheckZLevel::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("z", z);
}

void PieceNested::serialize( JsonOut &jsout ) const
{
    jsout.member("chunks", chunks);
    jsout.member("else_chunks", else_chunks);
    jsout.member("checks");
    jsout.start_array();
    for (const auto& it : checks) {
        jsout.start_object();
        jsout.member_as_string("type", it->get_type());
        it->serialize(jsout);
        jsout.end_object();
    }
    jsout.end_array();
}

void PieceNested::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read("chunks", chunks);
    jsin.read("else_chunks", else_chunks);
    TextJsonArray arr = jsin.get_array("checks");
    for (TextJsonObject obj : arr) {
        NestedCheckType type = obj.get_enum_value<NestedCheckType>("type");
        std::unique_ptr<NestedCheck> entry = NestedCheck::make(type);
        entry->deserialize(obj);
        checks.emplace_back(std::move(entry));
    }
}

void PieceAltTrap::serialize( JsonOut &jsout ) const
{
    jsout.member( "list", list );
}

void PieceAltTrap::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "list", list );
}

void PieceAltFurniture::serialize( JsonOut &jsout ) const
{
    jsout.member( "list", list );
}

void PieceAltFurniture::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "list", list );
}

void PieceAltTerrain::serialize( JsonOut &jsout ) const
{
    jsout.member( "list", list );
}

void PieceAltTerrain::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "list", list );
}

void PieceRemoveAll::serialize(JsonOut& jsout) const
{
    // No data
}

void PieceRemoveAll::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

void PieceUnknown::serialize(JsonOut& jsout) const
{

}

void PieceUnknown::deserialize(const JSON_OBJECT& jsin)
{

}

void SetMapTer::serialize(JsonOut& jsout) const
{
    jsout.member("id", id);
}

void SetMapTer::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("id", id);
}

void SetMapFurn::serialize(JsonOut& jsout) const
{
    jsout.member("id", id);
}

void SetMapFurn::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("id", id);
}

void SetMapTrap::serialize(JsonOut& jsout) const
{
    jsout.member("id", id);
}

void SetMapTrap::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("id", id);
}

void SetMapVariable::serialize(JsonOut& jsout) const
{
    jsout.member("id", id);
}

void SetMapVariable::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("id", id);
}

void SetMapBash::serialize(JsonOut& jsout) const
{
    // No data
}

void SetMapBash::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

void SetMapBurn::serialize(JsonOut& jsout) const
{
    // No data
}

void SetMapBurn::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

void SetMapRadiation::serialize(JsonOut& jsout) const
{
    jsout.member("amount", amount);
}

void SetMapRadiation::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("amount", amount);
}

void SetMapRemoveTrap::serialize(JsonOut& jsout) const
{
    jsout.member("id", id);
}

void SetMapRemoveTrap::deserialize(const JSON_OBJECT& jsin)
{
    jsin.read("id", id);
}

void SetMapRemoveCreature::serialize(JsonOut& jsout) const
{
    // No data
}

void SetMapRemoveCreature::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

void SetMapRemoveItem::serialize(JsonOut& jsout) const
{
    // No data
}

void SetMapRemoveItem::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

void SetMapRemoveField::serialize(JsonOut& jsout) const
{
    // No data
}

void SetMapRemoveField::deserialize(const JSON_OBJECT& jsin)
{
    // No data
}

namespace detail
{

void serialize_eid( JsonOut &jsout, const std::string &data )
{
    jsout.start_object();
    jsout.member( "data", data );
    jsout.end_object();
}

void deserialize_eid( const TextJsonValue &jsin, std::string &data )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "data", data );
}

} // namespace detail

void PaletteEntry::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "key", key );
    jsout.member( "color", color );
    jsout.member( "name", name );
    jsout.member( "pieces", pieces );
    jsout.end_object();
}

void PaletteEntry::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "key", key );
    jo.read( "color", color );
    jo.read( "name", name );
    jo.read( "pieces", pieces );
}

void Palette::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "imported", imported );
    jsout.member( "standalone", standalone );
    jsout.member( "temp_palette", temp_palette );
    jsout.member( "imported_id", imported_id );
    jsout.member( "created_id", created_id );
    jsout.member( "name", name );
    jsout.member( "entries", entries );
    jsout.end_object();
}

void Palette::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "uuid", uuid );
    jo.read( "imported", imported );
    jo.read( "standalone", standalone );
    jo.read( "temp_palette", temp_palette );
    jo.read( "imported_id", imported_id );
    jo.read( "created_id", created_id );
    jo.read( "name", name );
    jo.read( "entries", entries );
}

void MapKey::serialize(JsonOut& jsout) const
{
    jsout.write(value);
}

void MapKey::deserialize(const TextJsonValue& jsin)
{
    jsin.read(value);
}

void MapObject::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "x", x );
    jsout.member( "y", y );
    jsout.member( "repeat", repeat );
    jsout.member( "color", color );
    jsout.member( "visible", visible );
    jsout.member( "piece", piece );
    jsout.end_object();
}

void MapObject::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "x", x );
    jo.read( "y", y );
    jo.read( "repeat", repeat );
    jo.read( "color", color );
    jo.read( "visible", visible );
    jo.read( "piece", piece );
}

void SetMap::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member("uuid", uuid);
    jsout.member("x", x);
    jsout.member("y", y);
    jsout.member("x2", x2);
    jsout.member("y2", y2);
    jsout.member("z", z);
    jsout.member("chance", chance);
    jsout.member("repeat", repeat);
    jsout.member("color", color);
    jsout.member("visible", visible);
    jsout.member_as_string("mode", mode);
    jsout.member("data", data);
    jsout.end_object();
}

void SetMap::deserialize(const TextJsonObject& jo)
{
    jo.read("uuid", uuid);
    jo.read("x", x);
    jo.read("y", y);
    jo.read("x2", x2);
    jo.read("y2", y2);
    jo.read("z", z);
    jo.read("chance", chance);
    jo.read("repeat", repeat);
    jo.read("color", color);
    jo.read("visible", visible);
    jo.read("mode", mode);
    jo.read("data", data);
}

void MapgenBase::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "canvas", canvas );
    jsout.member( "inline_palette_id", palette );
    jsout.end_object();
}

void MapgenBase::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "canvas", canvas );
    jo.read( "inline_palette_id", palette );
}

void MapgenOter::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "matrix_mode", matrix_mode );
    jsout.member( "om_terrain", om_terrain );
    jsout.member( "om_terrain_matrix", om_terrain_matrix );
    jsout.member( "weight", weight );
    jsout.member_as_string( "mapgen_base", mapgen_base );
    jsout.member("uses_rows", uses_rows);
    jsout.member( "fill_ter", fill_ter );
    jsout.member( "predecessor_mapgen", predecessor_mapgen );
    jsout.member( "rotation", rotation );
    jsout.end_object();
}

void MapgenOter::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "matrix_mode", matrix_mode );
    jo.read( "om_terrain", om_terrain );
    jo.read( "om_terrain_matrix", om_terrain_matrix );
    jo.read( "weight", weight );
    jo.read( "mapgen_base", mapgen_base );
    jo.read("uses_rows", uses_rows);
    jo.read( "fill_ter", fill_ter );
    jo.read( "predecessor_mapgen", predecessor_mapgen );
    jo.read( "rotation", rotation );
}

void MapgenUpdate::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "update_mapgen_id", update_mapgen_id );
    jsout.member( "fill_ter", fill_ter );
    jsout.member( "uses_rows", uses_rows );
    jsout.end_object();
}

void MapgenUpdate::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "update_mapgen_id", update_mapgen_id );
    jo.read( "fill_ter", fill_ter );
    jo.read( "uses_rows", uses_rows );
}

void MapgenNested::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "nested_mapgen_id", nested_mapgen_id );
    jsout.member( "size", size );
    jsout.member( "rotation", rotation );
    jsout.end_object();
}

void MapgenNested::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "nested_mapgen_id", nested_mapgen_id );
    jo.read( "size", size );
    jo.read( "rotation", rotation );
}

void MapgenFlags::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member("avoid_creatures", avoid_creatures);
    jsout.member("no_underlying_rotate", no_underlying_rotate);
    jsout.member_as_string("rule_items", rule_items);
    jsout.member_as_string("rule_furn", rule_furn);
    jsout.member_as_string("rule_traps", rule_traps);
    jsout.end_object();
}

void MapgenFlags::deserialize(const TextJsonObject& jo)
{
    jo.read("avoid_creatures", avoid_creatures);
    jo.read("no_underlying_rotate", no_underlying_rotate);
    jo.read("rule_items", rule_items);
    jo.read("rule_furn", rule_furn);
    jo.read("rule_traps", rule_traps);
}

static int project_load_version_val = PROJECT_FORMAT_VERSION;

int project_load_version()
{
    return project_load_version_val;
}

void Mapgen::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "name", name );
    jsout.member_as_string( "mtype", mtype );
    jsout.member( "base", base );
    jsout.member( "oter", oter );
    jsout.member( "update", update );
    jsout.member( "nested", nested );
    jsout.member( "objects", objects );
    jsout.member( "setmaps", setmaps );
    jsout.member( "flags", flags );
    jsout.member( "selection_mask", selection_mask );
    jsout.end_object();
}

void Mapgen::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "uuid", uuid );
    jo.read( "name", name );
    jo.read( "mtype", mtype );
    jo.read( "base", base );
    jo.read( "oter", oter );
    jo.read( "update", update );
    jo.read( "nested", nested );
    jo.read( "objects", objects );
    jo.read( "setmaps", setmaps );
    jo.read( "flags", flags );
    jo.read( "selection_mask", selection_mask );

    for (const MapObject& obj : objects) {
        obj.piece->is_object = true;
    }
}

void Project::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "project_format_version", PROJECT_FORMAT_VERSION );
    jsout.member( "project_uuid", project_uuid );
    jsout.member( "uuid_gen", uuid_generator );
    jsout.member( "files", mapgens );
    jsout.member( "palettes", palettes );
    jsout.end_object();
}

void Project::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "project_format_version", project_load_version_val );
    jo.read( "project_uuid", project_uuid );
    jo.read( "uuid_gen", uuid_generator );
    jo.read( "files", mapgens );
    jo.read( "palettes", palettes );
}

void SelectionMask::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "data", data );
    jsout.end_object();
}

void SelectionMask::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "data", data );

    refresh_num_selected();
}

void CanvasSnippet::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "data", data );
    jsout.member( "mask", mask );
    jsout.member( "pos", pos );
    jsout.end_object();
}

void CanvasSnippet::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "data", data );
    jo.read( "mask", mask );
    jo.read( "pos", pos );
}

} // namespace editor
