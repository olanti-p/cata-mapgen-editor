#include "project_export.h"

#include "mapgen/mapgen.h"
#include "project.h"

#include "mapgen/palette.h"
#include "mapgen/piece_impl.h"

#include "json.h"
#include "../../tools/format/format.h"
#include "common/weighted_list.h"
#include "widget/editable_id.h"
#include "common/map_key.h"

#include <sstream>

namespace editor_export
{

/**
 * ============= EMIT DECLARATIONS =============
 */

void emit_key( JsonOut &jo, const std::string &key );

void emit_val( JsonOut &jo, int i );
void emit_val( JsonOut &jo, float f );
void emit_val( JsonOut &jo, bool b );
void emit_val( JsonOut &jo, const char *str );
void emit_val( JsonOut &jo, const std::string &str );
//void emit_val( JsonOut &jo, const editor::MapKey &mk );
void emit_val( JsonOut &jo, const editor::Piece *piece );
void emit_val( JsonOut &jo, const editor::MapObject *obj );
template<typename T>
void emit_val( JsonOut &jo, const editor::EditableID<T> &eid );
void emit_val( JsonOut &jo, const editor::IntRange &r );
void emit_val( JsonOut &jo, const editor::ComputerOption &c );
void emit_val( JsonOut &jo, const editor::ComputerFailure &c );
template<typename T>
void emit_val( JsonOut &jo, const editor::WeightedList<T> &list );

template<typename T>
void emit( JsonOut &jo, const std::string &key, T value );

template<typename F>
void emit_array( JsonOut &jo, F func );
template<typename F>
void emit_array( JsonOut &jo, const std::string &key, F func );
template<typename T>
void emit_array( JsonOut &jo, const std::vector<T>& vals );

template<typename F>
void emit_object( JsonOut &jo, F func );
template<typename F>
void emit_object( JsonOut &jo, const std::string &key, F func );

template<typename T>
void emit_single_or_array( JsonOut &jo, const std::vector<T> &vals );
template<typename T>
void emit_single_or_array( JsonOut &jo, const std::string &key, const std::vector<T> &vals );

/**
 * ============= EMIT DEFINITIONS =============
 */

void emit_key( JsonOut &jo, const std::string &key )
{
    jo.member( key );
}

void emit_val( JsonOut &jo, int i )
{
    jo.write( i );
}

void emit_val( JsonOut &jo, float f )
{
    jo.write( f );
}

void emit_val( JsonOut &jo, bool b )
{
    jo.write_bool( b );
}

void emit_val( JsonOut &jo, const char *str )
{
    jo.write( str );
}

void emit_val( JsonOut &jo, const std::string &str )
{
    jo.write( str );
}

//void emit_val( JsonOut& jo, const editor::MapKey& mk )
//{
//    jo.write( mk.str() );
//}

void emit_val( JsonOut &jo, const editor::Piece *piece )
{
    if( editor::is_alt_piece( piece->get_type() ) ) {
        piece->export_func( jo );
    } else {
        emit_object( jo, [&]() {
            piece->export_func( jo );
        } );
    }
}

void emit_val( JsonOut &jo, const editor::MapObject *obj )
{
    emit_object( jo, [&]() {
        obj->piece->export_func( jo );
        emit( jo, "x", obj->x );
        emit( jo, "y", obj->y );
        if (obj->repeat.min != 1 || obj->repeat.max != 1) {
            emit( jo, "repeat", obj->repeat );
        }
    } );
}

template<typename T>
void emit_val( JsonOut &jo, const editor::EditableID<T> &eid )
{
    jo.write( eid.data );
}

void emit_val( JsonOut &jo, const editor::IntRange &r )
{
    if( r.min == r.max ) {
        emit_val( jo, r.min );
    } else {
        emit_array( jo, [&]() {
            emit_val( jo, r.min );
            emit_val( jo, r.max );
        } );
    }
}

void emit_val(JsonOut& jo, const editor::ComputerOption& c)
{
    emit_object(jo, [&]() {
        emit(jo, "name", c.name);
        emit(jo, "action", io::enum_to_string( c.action ) );
        if (c.security != 0) {
            emit(jo, "security", c.security);
        }
    } );
}

void emit_val(JsonOut& jo, const editor::ComputerFailure& c)
{
    emit_object(jo, [&]() {
        emit(jo, "action", io::enum_to_string(c.action) );
    } );
}

template<typename T>
void emit_val( JsonOut &jo, const editor::WeightedList<T> &list )
{
    if( list.entries.size() == 1 ) {
        emit_val( jo, list.entries[0].val );
    } else {
        emit_array( jo, [&]() {
            for( const auto &e : list.entries ) {
                if( e.weight == 1 ) {
                    emit_val( jo, e.val );
                } else {
                    emit_array( jo, [&]() {
                        emit_val( jo, e.val );
                        emit_val( jo, e.weight );
                    } );
                }
            }
        } );
    }
}

template<typename T>
void emit( JsonOut &jo, const std::string &key, T value )
{
    emit_key( jo, key );
    emit_val( jo, value );
}

template<typename F>
void emit_array( JsonOut &jo, F func )
{
    jo.start_array();
    func();
    jo.end_array();
}

template<typename F>
void emit_array( JsonOut &jo, const std::string &key, F func )
{
    emit_key( jo, key );
    emit_array( jo, func );
}

template<typename T>
void emit_array( JsonOut &jo, const std::vector<T>& vals )
{
    emit_array( jo, [&]() {
        for( const T &itm : vals ) {
            emit_val( jo, itm );
        }
    } );
}

template<typename F>
void emit_object( JsonOut &jo, F func )
{
    jo.start_object();
    func();
    jo.end_object();
}

template<typename F>
void emit_object( JsonOut &jo, const std::string &key, F func )
{
    emit_key( jo, key );
    emit_object( jo, func );
}

template<typename T>
void emit_single_or_array( JsonOut &jo, const std::vector<T> &vals )
{
    if( vals.size() == 1 ) {
        emit_val( jo, vals[0] );
    } else {
        emit_array( jo, [&]() {
            for( const T &itm : vals ) {
                emit_val( jo, itm );
            }
        } );
    }
}

template<typename T>
void emit_single_or_array( JsonOut &jo, const std::string &key, const std::vector<T> &vals )
{
    emit_key( jo, key );
    emit_single_or_array( jo, vals );
}

} // namespace editor_export

/**
 * ==================== PIECES ====================
 */

namespace editor
{

namespace ee = editor_export;

void PieceField::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "field", ftype );
    if (remove) {
        ee::emit(jo, "remove", true);
    }
    else {
        std::vector<int> ints;
        if (int_1) {
            ints.push_back(1);
        }
        if (int_2) {
            ints.push_back(2);
        }
        if (int_3) {
            ints.push_back(3);
        }
        if (ints.empty()) {
            // May happen if user leaves invalid selection
            ints.push_back(1);
        }
        ee::emit_single_or_array(jo, "intensity", ints);
        if( age != 0_turns ) {
            ee::emit( jo, "age", to_turns<int>( age ) );
        }
    }
    if (chance != 100) {
        ee::emit(jo, "chance", chance);
    }
}

void PieceNPC::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "class", npc_class );
    if( target ) {
        ee::emit( jo, "target", target );
    }
    if (!unique_id.empty()) {
        ee::emit(jo, "unique_id", unique_id);
    }
    if( !traits.empty() ) {
        ee::emit_single_or_array( jo, "add_trait", traits );
    }
}

void PieceFaction::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "id", id );
}

void PieceSign::export_func( JsonOut &jo ) const
{
    if( use_snippet ) {
        ee::emit( jo, "snippet", snippet );
    } else {
        ee::emit( jo, "signage", text );
    }
    if (furniture != default_furniture) {
        ee::emit(jo, "furniture", furniture);
    }
}

void PieceGraffiti::export_func( JsonOut &jo ) const
{
    if( use_snippet ) {
        ee::emit( jo, "snippet", snippet );
    } else {
        ee::emit( jo, "text", text );
    }
}

void PieceVendingMachine::export_func( JsonOut &jo ) const
{
    if( lootable ) {
        ee::emit( jo, "lootable", lootable );
    }
    if( powered ) {
        ee::emit( jo, "powered", powered );
    }
    if( networked ) {
        ee::emit( jo, "networked", networked );
    }
    if( reinforced ) {
        ee::emit( jo, "reinforced", reinforced );
    }
    if( !use_default_group ) {
        ee::emit( jo, "item_group", item_group );
    }
}

void PieceToilet::export_func( JsonOut &jo ) const
{
    if( !use_default_amount ) {
        ee::emit( jo, "amount", amount );
    }
}

void PieceGaspump::export_func( JsonOut &jo ) const
{
    if( !use_default_amount ) {
        ee::emit( jo, "amount", amount );
    }
    if( fuel == GasPumpFuel::Gasoline ) {
        ee::emit( jo, "fuel", "gasoline" );
    } else if( fuel == GasPumpFuel::Diesel ) {
        ee::emit( jo, "fuel", "diesel" );
    } else if( fuel == GasPumpFuel::JP8 ) {
        ee::emit( jo, "fuel", "jp8" );
    } else if( fuel == GasPumpFuel::AvGas ) {
        ee::emit( jo, "fuel", "avgas" );
    }
}

void PieceLiquid::export_func( JsonOut &jo ) const
{
    if( !use_default_amount ) {
        ee::emit( jo, "amount", amount );
    }
    ee::emit( jo, "liquid", liquid );
    if( !spawn_always ) {
        ee::emit( jo, "chance", chance );
    }
}

void PieceIGroup::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "item", group_id );
    ee::emit( jo, "chance", chance );
    if( !is_object && !spawn_once ) {
        ee::emit( jo, "repeat", repeat );
    }
    if( faction != default_faction ) {
        ee::emit( jo, "faction", faction );
    }
}

void PieceLoot::export_func( JsonOut &jo ) const
{
    // TODO
}

void PieceMGroup::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "monster", group_id );
    if( !spawn_always ) {
        ee::emit( jo, "chance", chance );
    }
    if( !use_default_density ) {
        ee::emit( jo, "density", density );
    }
}

void PieceMonster::export_func( JsonOut &jo ) const
{
    if (use_mongroup) {
        ee::emit(jo,"group", group_id);
    }
    else {
        ee::emit(jo, "monster", type_list);
    }
    if (chance.min != 100 || chance.max != 100) {
        ee::emit(jo, "chance", chance);
    }
}

void PieceVehicle::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "vehicle", group_id );
    ee::emit( jo, "chance", chance );
    if( status == VehicleStatus::Undamaged ) {
        ee::emit( jo, "status", 0 );
    } else if( status == VehicleStatus::Disabled ) {
        ee::emit( jo, "status", 1 );
    }
    else if (status == VehicleStatus::Pristine) {
        ee::emit(jo, "status", 2);
    }
    if( !random_fuel_amount ) {
        ee::emit( jo, "fuel", fuel );
    }
    std::vector<int> rots;
    rots.reserve( allowed_rotations.size() );
    for( int r : allowed_rotations ) {
        rots.push_back( r );
    }
    if( !rots.empty() ) {
        ee::emit_single_or_array( jo, "rotation", rots );
    }
    if (faction != default_faction) {
        ee::emit(jo, "faction", faction);
    }
}

void PieceItem::export_func( JsonOut &jo ) const
{
    ee::emit( jo, "item", item_id );
    if (!variant.empty()) {
        ee::emit( jo, "variant", variant );
    }
    if( amount.min != 1 || amount.max != 1 ) {
        ee::emit( jo, "amount", amount );
    }
    if (chance.min != 100 || chance.max != 100) {
        ee::emit(jo, "chance", chance);
    }
    if ( !is_object && ( repeat.min != 1 || repeat.max != 1) ) {
        ee::emit(jo, "repeat", repeat);
    }
    if (!custom_flags.empty()) {
        ee::emit_array(jo, "custom-flags", custom_flags);
    }
    if (faction != default_faction) {
        ee::emit(jo, "faction", faction);
    }
}

void PieceTrap::export_func( JsonOut &jo ) const
{
    if (remove) {
        ee::emit(jo, "remove", true);
    }
    else {
        ee::emit(jo, "trap", id);
    }
}

void PieceFurniture::export_func( JsonOut &jo ) const
{
    ee::emit(jo, "furn", id);
}

void PieceTerrain::export_func( JsonOut &jo ) const
{
    ee::emit(jo, "ter", id);
}

void PieceTerFurnTransform::export_func( JsonOut &jo ) const
{
    ee::emit(jo, "transform", id);
}

void PieceMakeRubble::export_func( JsonOut &jo ) const
{
    if (rubble_type != default_rubble_type) {
        ee::emit(jo, "rubble_type", rubble_type);
    }
    if (floor_type != default_floor_type) {
        ee::emit(jo, "floor_type", floor_type);
    }
    if (items) {
        ee::emit(jo, "items", items);
    }
    if (overwrite) {
        ee::emit(jo, "overwrite", overwrite);
    }
}

void PieceComputer::export_func( JsonOut &jo ) const
{
    if (!name.empty()) {
        ee::emit(jo, "name", name);
    }
    if (!access_denied.empty()) {
        ee::emit(jo, "access_denied", access_denied);
    }
    if (security != 0) {
        ee::emit(jo, "security", security);
    }
    if (target) {
        ee::emit(jo, "target", target);
    }
    if (!options.empty()) {
        ee::emit_array(jo, "options", options);
    }
    if (!failures.empty()) {
        ee::emit_array(jo, "failures", failures);
    }
    if (!chat_topics.empty()) {
        ee::emit_array(jo, "chat_topics", chat_topics);
    }
    if (!eocs.empty()) {
        ee::emit_array(jo, "eocs", eocs);
    }
}

void PieceSealeditem::export_func( JsonOut &jo ) const
{
    // TODO
}

void PieceZone::export_func( JsonOut &jo ) const
{
    ee::emit(jo, "type", zone_type);
    ee::emit(jo, "faction", faction);
    if (!name.empty()) {
        ee::emit(jo, "name", name);
    }
    if (!filter.empty()) {
        ee::emit(jo, "filter", filter);
    }
}

void PieceNested::export_func( JsonOut &jo ) const
{
    // TODO
}

void PieceAltTrap::export_func( JsonOut &jo ) const
{
    ee::emit_val( jo, list );
}

void PieceAltFurniture::export_func( JsonOut &jo ) const
{
    ee::emit_val( jo, list );
}

void PieceAltTerrain::export_func( JsonOut &jo ) const
{
    ee::emit_val( jo, list );
}

void PieceRemoveAll::export_func(JsonOut& jo) const
{
    // Empty object
}

void PieceUnknown::export_func(JsonOut& jo) const
{
    // TODO: print some error or something
}

} // namespace editor

namespace editor_export
{

std::string get_palette_category( editor::PieceType data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::PieceType::Field: return "fields";
        case editor::PieceType::NPC: return "npcs";
        case editor::PieceType::Faction: return "faction_owner_character";
        case editor::PieceType::Sign: return "signs";
        case editor::PieceType::Graffiti: return "graffiti";
        case editor::PieceType::VendingMachine: return "vendingmachines";
        case editor::PieceType::Toilet: return "toilets";
        case editor::PieceType::GasPump: return "gaspumps";
        case editor::PieceType::Liquid: return "liquids";
        case editor::PieceType::IGroup: return "items";
        case editor::PieceType::MGroup: return "monsters";
        case editor::PieceType::Monster: return "monster";
        case editor::PieceType::Vehicle: return "vehicles";
        case editor::PieceType::Item: return "item";
        case editor::PieceType::TerFurnTransform: return "ter_furn_transforms";
        case editor::PieceType::MakeRubble: return "rubble";
        case editor::PieceType::Computer: return "computers";
        case editor::PieceType::SealedItem: return "sealed_item";
        case editor::PieceType::Zone: return "zones";
        case editor::PieceType::Nested: return "nested";
        case editor::PieceType::AltTrap: return "trap";
        case editor::PieceType::AltFurniture: return "furniture";
        case editor::PieceType::AltTerrain: return "terrain";
        case editor::PieceType::RemoveAll: return "remove_all";
        // These cannot exist as mappings
        case editor::PieceType::Unknown:
        case editor::PieceType::Loot:
        case editor::PieceType::Trap:
        case editor::PieceType::Furniture:
        case editor::PieceType::Terrain: {
            return "";
        }
        // *INDENT-ON*
        default:
            break;
    }
    debugmsg( "Invalid editor::PieceType" );
    abort();
}

std::string get_object_category( editor::PieceType data )
{
    switch( data ) {
        // *INDENT-OFF*
        case editor::PieceType::Field: return "place_fields";
        case editor::PieceType::NPC: return "place_npcs";
        case editor::PieceType::Faction: return "faction_owner";
        case editor::PieceType::Sign: return "place_signs";
        case editor::PieceType::Graffiti: return "place_graffiti";
        case editor::PieceType::VendingMachine: return "place_vendingmachines";
        case editor::PieceType::Toilet: return "place_toilets";
        case editor::PieceType::GasPump: return "place_gaspumps";
        case editor::PieceType::Liquid: return "place_liquids";
        case editor::PieceType::IGroup: return "place_items";
        case editor::PieceType::Loot: return "place_loot";
        case editor::PieceType::MGroup: return "place_monsters";
        case editor::PieceType::Monster: return "place_monster";
        case editor::PieceType::Vehicle: return "place_vehicles";
        case editor::PieceType::Item: return "place_item";
        case editor::PieceType::Trap: return "place_traps";
        case editor::PieceType::Furniture: return "place_furniture";
        case editor::PieceType::Terrain: return "place_terrain";
        case editor::PieceType::TerFurnTransform: return "place_ter_furn_transforms";
        case editor::PieceType::MakeRubble: return "place_rubble";
        case editor::PieceType::Computer: return "place_computers";
        case editor::PieceType::Zone: return "place_zones";
        case editor::PieceType::Nested: return "place_nested";
        // These cannot exist as objects
        case editor::PieceType::RemoveAll:
        case editor::PieceType::Unknown:
        case editor::PieceType::SealedItem:
        case editor::PieceType::AltTrap:
        case editor::PieceType::AltFurniture:
        case editor::PieceType::AltTerrain: {
            return "";
        }
        // *INDENT-ON*
        default:
            break;
    }
    debugmsg( "Invalid editor::PieceType" );
    abort();
}

/**
 * ============= HIGH-LEVEL FUNCTIONS =============
 */

using key_pieces_pair = std::pair<editor::MapKey, std::vector<const editor::Piece*>>;

static void emit_palette_entries(JsonOut& jo, const editor::Palette& pal)
{
    if (!pal.ancestors.list.empty()) {
        emit_array(jo, "palettes", [&]() {
            for (const auto& sw : pal.ancestors.list) {
                emit_val(jo, sw.options[0]);
            }
        });
    }

    for (const auto& it : editor::get_piece_templates()) {
        editor::PieceType pt = it->get_type();

        std::string palette_cat = get_palette_category(pt);

        std::vector<key_pieces_pair> matching_pieces;

        for (const editor::PaletteEntry& entry : pal.entries) {
            for (const auto& piece : entry.pieces) {
                if (piece->get_type() == pt) {
                    bool found_bucket = false;
                    for (key_pieces_pair& it : matching_pieces) {
                        if (it.first == entry.key) {
                            it.second.push_back(piece.get());
                            found_bucket = true;
                            break;
                        }
                    }
                    if (!found_bucket) {
                        std::vector< const editor::Piece*> pieces;
                        pieces.emplace_back(piece.get());
                        matching_pieces.emplace_back(entry.key, std::move(pieces));
                    }
                }
            }
        }

        if (matching_pieces.empty()) {
            // Nothing to do
            continue;
        }

        if (palette_cat.empty()) {
            std::cerr << string_format(
                "Tried to export piece of type %s as a mapping.",
                io::enum_to_string(pt)
            );
            std::abort();
        }

        emit_object(jo, palette_cat, [&]() {
            for (const key_pieces_pair& it : matching_pieces) {
                emit_single_or_array(jo, it.first.str(), it.second);
            }
        });
    }
}

static void emit_palette_contents(JsonOut& jo, const editor::Project& project,
    const editor::Palette& palette)
{
    emit(jo, "type", "palette");
    emit(jo, "id", palette.created_id);
    emit_palette_entries(jo, palette);
}

static void emit_mapgen_contents( JsonOut &jo, const editor::Project &project,
                                  const editor::Mapgen &mapgen )
{
    emit( jo, "type", "mapgen" );

    if( mapgen.mtype == editor::MapgenType::Oter ) {
        if( !mapgen.oter.matrix_mode ) {
            emit_single_or_array( jo, "om_terrain", mapgen.oter.om_terrain );
        } else {
            point size = mapgen.oter.om_terrain_matrix.get_size();
            emit_array( jo, "om_terrain", [&]() {
                for( int y = 0; y < size.y; y++ ) {
                    emit_array( jo, [&]() {
                        for( int x = 0; x < size.x; x++ ) {
                            emit_val( jo, mapgen.oter.om_terrain_matrix.get( point( x, y ) ) );
                        }
                    } );
                }
            } );
        }
        if( mapgen.oter.weight != editor::DEFAULT_OTER_MAPGEN_WEIGHT ) {
            emit( jo, "weight", mapgen.oter.weight );
        }
    } else if( mapgen.mtype == editor::MapgenType::Nested ) {
        emit( jo, "nested_mapgen_id", mapgen.nested.nested_mapgen_id );
    } else { // editor::MapgenType::Update
        emit( jo, "update_mapgen_id", mapgen.update.update_mapgen_id );
    }
    emit(jo, "method", "json");

    emit_object( jo, "object", [&]() {

        if( mapgen.mtype == editor::MapgenType::Oter ) {
            if( mapgen.oter.mapgen_base == editor::OterMapgenBase::PredecessorMapgen ) {
                emit( jo, "predecessor_mapgen", mapgen.oter.predecessor_mapgen );
            } else if( !mapgen.oter.fill_ter.is_null() ) {
                emit( jo, "fill_ter", mapgen.oter.fill_ter );
            }
            if( mapgen.oter.rotation ) {
                emit( jo, "rotation", mapgen.oter.rotation );
            }
        } else if( mapgen.mtype == editor::MapgenType::Nested ) {
            emit_array( jo, "mapgensize", [&]() {
                emit_val( jo, mapgen.nested.size.x );
                emit_val( jo, mapgen.nested.size.y );
            } );
            if( mapgen.nested.rotation ) {
                emit( jo, "rotation", mapgen.nested.rotation );
            }
        } else { // editor::MapgenType::Update
            if( !mapgen.update.fill_ter.is_null() ) {
                emit( jo, "fill_ter", mapgen.update.fill_ter );
            }
        }

        if( mapgen.uses_rows() ) {
            const editor::Palette &pal = *project.get_palette( mapgen.base.palette );
            emit_array( jo, "rows", [&]() {
                const editor::Canvas2D<editor::MapKey> &canvas = mapgen.base.canvas;
                for( int y = 0; y < canvas.get_size().y; y++ ) {
                    std::string s;
                    for( int x = 0; x < canvas.get_size().x; x++ ) {
                        const editor::MapKey &mk = canvas.get( point( x, y ) );
                        s += mk.str();
                    }
                    emit_val( jo, s );
                }
            } );

            if (pal.imported && !pal.temp_palette) {
                emit_array(jo, "palettes", [&]() {
                    emit_val(jo, pal.imported_id);
                });
            }
            else if (!pal.imported && pal.standalone) {
                emit_array(jo, "palettes", [&]() {
                    emit_val(jo, pal.created_id);
                });
            }
            else {
                emit_palette_entries(jo, pal);
            }
        }

        for( const auto &it : editor::get_piece_templates() ) {
            editor::PieceType pt = it->get_type();

            std::string object_cat = get_object_category( pt );

            std::vector<const editor::MapObject *> matching_objects;

            for( const editor::MapObject &it : mapgen.objects ) {
                if( it.piece->get_type() == pt ) {
                    matching_objects.push_back( &it );
                }
            }

            if( matching_objects.empty() || pt == editor::PieceType::Unknown ) {
                // Nothing to do
                continue;
            }

            if( object_cat.empty() ) {
                std::cerr << string_format(
                              "Tried to export piece of type %s as an object.",
                              io::enum_to_string( pt )
                          );
                std::abort();
            }

            emit_array( jo, object_cat, [&]() {
                for( const editor::MapObject *obj : matching_objects ) {
                    emit_val( jo, obj );
                }
            } );
        }
    } );
}

std::string to_string( const editor::Project &project )
{
    return serialize_wrapper( [&]( JsonOut & jo ) {
        emit_array( jo, [&]() {
            for (const editor::Palette& palette : project.palettes) {
                if (palette.imported || !palette.standalone) {
                    continue;
                }
                emit_object(jo, [&]() {
                    emit_palette_contents(jo, project, palette);
                });
            }
            for( const editor::Mapgen &mapgen : project.mapgens ) {
                emit_object( jo, [&]() {
                    emit_mapgen_contents( jo, project, mapgen );
                } );
            }
        } );
    } );
}

std::string format_string( const std::string &js )
{
    std::stringstream in;
    std::stringstream out;

    in << js;

    JsonOut jsout( out, true );
    JSON_IN jsin( in );

    formatter::format( jsin, jsout );

    out << std::endl;

    return out.str();
}

} // namespace editor_export
