#include "project_serde.h"

#include "common/canvas_2d_serde.h"
#include "common/weighted_list_serde.h"
#include "imgui.h"
#include "json.h"
#include "mapgen.h"
#include "mapgen/mapgen.h"
#include "mapgen/piece_impl.h"
#include "mapgen/piece_type.h"
#include "mapgen/selection_mask.h"
#include "project.h"

void serialize( const std::unique_ptr<editor::Piece> &ptr, JsonOut &jsout )
{
    jsout.start_object();
    jsout.member_as_string( "piece_type", ptr->get_type() );
    jsout.member( "uuid", ptr->uuid );
    ptr->serialize( jsout );
    jsout.end_object();
}

void deserialize( std::unique_ptr<editor::Piece> &ptr, const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    editor::PieceType pt = editor::PieceType::NumJmTypes;
    jo.read( "piece_type", pt );
    editor::UUID uuid;
    jo.read( "uuid", uuid );

    std::unique_ptr<editor::Piece> val = editor::make_new_piece( pt );
    val->deserialize( jo );
    val->uuid = uuid;
    ptr = std::move( val );
}

void serialize( const map_key &mk, JsonOut &jsout )
{
    jsout.start_object();
    jsout.member( "str", mk.str );
    jsout.end_object();
}

void deserialize( map_key &mk, const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "str", mk.str );
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
std::string enum_to_string<editor::OterMapgenBase>( editor::OterMapgenBase data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::OterMapgenBase::FillTer: return "FillTer";
        case editor::OterMapgenBase::PredecessorMapgen: return "PredecessorMapgen";
        case editor::OterMapgenBase::Rows: return "Rows";
        // *INDENT-ON*
        case editor::OterMapgenBase::_Num:
            break;
    }
    debugmsg( "Invalid editor::OterMapgenBase" );
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
        // *INDENT-ON*
        case editor::VehicleStatus::_Num:
            break;
    }
    debugmsg( "Invalid editor::VehicleStatus" );
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
        case editor::PieceType::Translate: return "Translate";
        case editor::PieceType::Zone: return "Zone";
        case editor::PieceType::Nested: return "Nested";
        case editor::PieceType::AltTrap: return "AltTrap";
        case editor::PieceType::AltFurniture: return "AltFurniture";
        case editor::PieceType::AltTerrain: return "AltTerrain";
        // *INDENT-ON*
        default:
            break;
    }
    debugmsg( "Invalid editor::PieceType" );
    abort();
}

} // namespace io

namespace editor
{

void PieceField::serialize( JsonOut &jsout ) const
{
    jsout.member( "ftype", ftype );
    jsout.member( "intensity", intensity );
    jsout.member( "age", age );
}

void PieceField::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "ftype", ftype );
    jsin.read( "intensity", intensity );
    jsin.read( "age", age );
}

void PieceNPC::serialize( JsonOut &jsout ) const
{
    jsout.member( "npc_class", npc_class );
    jsout.member( "target", target );
    jsout.member( "traits", traits );
}

void PieceNPC::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "npc_class", npc_class );
    jsin.read( "target", target );
    jsin.read( "traits", traits );
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
}

void PieceSign::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "use_snippet", use_snippet );
    jsin.read( "snippet", snippet );
    jsin.read( "text", text );
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
    jsout.member( "reinforced", reinforced );
    jsout.member( "use_default_group", use_default_group );
    jsout.member( "item_group", item_group );
}

void PieceVendingMachine::deserialize( const JSON_OBJECT &jsin )
{
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
}

void PieceIGroup::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "chance", chance );
    jsin.read( "group_id", group_id );
    jsin.read( "spawn_once", spawn_once );
    jsin.read( "repeat", repeat );
}

void PieceLoot::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceLoot::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
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
    // TODO
}

void PieceMonster::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceVehicle::serialize( JsonOut &jsout ) const
{
    jsout.member( "group_id", group_id );
    jsout.member( "chance", chance );
    jsout.member_as_string( "status", status );
    jsout.member( "random_fuel_amount", random_fuel_amount );
    jsout.member( "fuel", fuel );
    jsout.member( "allowed_rotations", allowed_rotations );
}

void PieceVehicle::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "group_id", group_id );
    jsin.read( "chance", chance );
    jsin.read( "status", status );
    jsin.read( "random_fuel_amount", random_fuel_amount );
    jsin.read( "fuel", fuel );
    jsin.read( "allowed_rotations", allowed_rotations );
}

void PieceItem::serialize( JsonOut &jsout ) const
{
    jsout.member( "item_id", item_id );
    jsout.member( "amount", amount );
    jsout.member( "spawn_one", spawn_one );
    jsout.member( "chance", chance );
    jsout.member( "spawn_once", spawn_once );
    jsout.member( "repeat", repeat );
}

void PieceItem::deserialize( const JSON_OBJECT &jsin )
{
    jsin.read( "item_id", item_id );
    jsin.read( "amount", amount );
    jsin.read( "spawn_one", spawn_one );
    jsin.read( "chance", chance );
    jsin.read( "spawn_once", spawn_once );
    jsin.read( "repeat", repeat );
}

void PieceTrap::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceTrap::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceFurniture::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceFurniture::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceTerrain::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceTerrain::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceTerFurnTransform::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceTerFurnTransform::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceMakeRubble::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceMakeRubble::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceComputer::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceComputer::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceSealeditem::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceSealeditem::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceTranslate::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceTranslate::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceZone::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceZone::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
}

void PieceNested::serialize( JsonOut &jsout ) const
{
    // TODO
}

void PieceNested::deserialize( const JSON_OBJECT &jsin )
{
    // TODO
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

void Mapping::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "pieces", pieces );
    jsout.end_object();
}

void Mapping::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "pieces", pieces );
}

void PaletteEntry::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "key", key );
    jsout.member( "color", color );
    jsout.member( "name", name );
    jsout.member( "mapping", mapping );
    jsout.end_object();
}

void PaletteEntry::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "key", key );
    jo.read( "color", color );
    jo.read( "name", name );
    jo.read( "mapping", mapping );
}

void Palette::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "id", id );
    jsout.member( "imported", imported );
    jsout.member( "inherits_from", inherits_from );
    jsout.member( "name", name );
    jsout.member( "entries", entries );
    jsout.end_object();
}

void Palette::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "uuid", uuid );
    jo.read( "id", id );
    jo.read( "imported", imported );
    jo.read( "inherits_from", inherits_from );
    jo.read( "name", name );
    jo.read( "entries", entries );
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
    TextJsonObject obj = jo.get_object("piece");
    piece->deserialize(obj);
    //jo.read( "piece", piece );
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
    jo.read( "fill_ter", fill_ter );
    jo.read( "predecessor_mapgen", predecessor_mapgen );
    jo.read( "rotation", rotation );
}

void MapgenUpdate::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "update_mapgen_id", update_mapgen_id );
    jsout.member( "fill_ter", fill_ter );
    jsout.end_object();
}

void MapgenUpdate::deserialize( const TextJsonValue &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "update_mapgen_id", update_mapgen_id );
    jo.read( "fill_ter", fill_ter );
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
    jo.read( "selection_mask", selection_mask );
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
