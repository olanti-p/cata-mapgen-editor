#ifndef CATA_SRC_EDITOR_PIECE_IMPL_H
#define CATA_SRC_EDITOR_PIECE_IMPL_H

#include "piece.h"

#include "calendar.h"
#include "enum_traits.h"
// FIXME: conflicts with include paths
#include "editor/widget/editable_id.h"
#include "editor/common/int_range.h"
#include "editor/common/weighted_list.h"

#include <set>

namespace editor
{

struct PieceField : public Piece {
    IMPLEMENT_ME_PIECE( PieceField, PieceType::Field )

    EID::Field ftype;
    int intensity = 1;
    time_duration age = 0_seconds;
};

struct PieceNPC : public Piece {
    IMPLEMENT_ME_PIECE( PieceNPC, PieceType::NPC )

    EID::NPCTemplate npc_class;
    bool target = false;
    std::vector<EID::Trait> traits;
};

struct PieceFaction : public Piece {
    IMPLEMENT_ME_PIECE( PieceFaction, PieceType::Faction )

    std::string id;
};

struct PieceSign : public Piece {
    IMPLEMENT_ME_PIECE( PieceSign, PieceType::Sign )

    bool use_snippet = false;
    EID::SnippetCategory snippet;
    std::string text;
};

struct PieceGraffiti : public Piece {
    IMPLEMENT_ME_PIECE( PieceGraffiti, PieceType::Graffiti )

    bool use_snippet = false;
    EID::SnippetCategory snippet;
    std::string text;
};

struct PieceVendingMachine : public Piece {
    IMPLEMENT_ME_PIECE( PieceVendingMachine, PieceType::VendingMachine )

    bool reinforced = false;
    bool use_default_group = true;
    EID::IGroup item_group;
};

struct PieceToilet : public Piece {
    IMPLEMENT_ME_PIECE( PieceToilet, PieceType::Toilet )

    bool use_default_amount = true;
    IntRange amount;
};

enum class GasPumpFuel {
    Random,
    Gasoline,
    Diesel,

    _Num,
};

struct PieceGaspump : public Piece {
    IMPLEMENT_ME_PIECE( PieceGaspump, PieceType::GasPump )

    bool use_default_amount = true;
    IntRange amount;
    GasPumpFuel fuel;
};

struct PieceLiquid : public Piece {
    IMPLEMENT_ME_PIECE( PieceLiquid, PieceType::Liquid )

    bool use_default_amount = true;
    IntRange amount;
    EID::Liquid liquid;
    bool spawn_always = true;
    IntRange chance;
};

struct PieceIGroup : public Piece {
    IMPLEMENT_ME_PIECE( PieceIGroup, PieceType::IGroup )

    EID::IGroup group_id;
    IntRange chance;
    bool spawn_once = true;
    IntRange repeat;
};

struct PieceLoot : public Piece {
    IMPLEMENT_ME_PIECE( PieceLoot, PieceType::Loot )

    // TODO
};

struct PieceMGroup : public Piece {
    IMPLEMENT_ME_PIECE( PieceMGroup, PieceType::MGroup )

    EID::MGroup group_id;
    bool spawn_always = true;
    IntRange chance;
    bool use_default_density = true;
    float density = -1.0f;
};

struct PieceMonster : public Piece {
    IMPLEMENT_ME_PIECE( PieceMonster, PieceType::Monster )

    // TODO
};

enum class VehicleStatus {
    LightDamage,
    Undamaged,
    Disabled,

    _Num,
};

struct PieceVehicle : public Piece {
    IMPLEMENT_ME_PIECE( PieceVehicle, PieceType::Vehicle )

    EID::VGroup group_id;
    IntRange chance;
    VehicleStatus status = VehicleStatus::LightDamage;
    bool random_fuel_amount = true;
    int fuel = 100;
    std::set<int> allowed_rotations;
};

struct PieceItem : public Piece {
    IMPLEMENT_ME_PIECE( PieceItem, PieceType::Item )

    EID::Item item_id;
    IntRange amount;
    bool spawn_one = true;
    IntRange chance;
    bool spawn_once = true;
    IntRange repeat;
};

struct PieceTrap : public Piece {
    IMPLEMENT_ME_PIECE( PieceTrap, PieceType::Trap )

    // TODO
};

struct PieceFurniture : public Piece {
    IMPLEMENT_ME_PIECE( PieceFurniture, PieceType::Furniture )

    // TODO
};

struct PieceTerrain : public Piece {
    IMPLEMENT_ME_PIECE( PieceTerrain, PieceType::Terrain )

    // TODO
};

struct PieceTerFurnTransform : public Piece {
    IMPLEMENT_ME_PIECE( PieceTerFurnTransform, PieceType::TerFurnTransform )

    // TODO
};

struct PieceMakeRubble : public Piece {
    IMPLEMENT_ME_PIECE( PieceMakeRubble, PieceType::MakeRubble )

    // TODO
};

struct PieceComputer : public Piece {
    IMPLEMENT_ME_PIECE( PieceComputer, PieceType::Computer )

    // TODO
};

struct PieceSealeditem : public Piece {
    IMPLEMENT_ME_PIECE( PieceSealeditem, PieceType::SealedItem )

    // TODO
};

struct PieceTranslate : public Piece {
    IMPLEMENT_ME_PIECE( PieceTranslate, PieceType::Translate )

    // TODO
};

struct PieceZone : public Piece {
    IMPLEMENT_ME_PIECE( PieceZone, PieceType::Zone )

    // TODO
};

struct PieceNested : public Piece {
    IMPLEMENT_ME_PIECE( PieceNested, PieceType::Nested )

    void init_new() override;

    WeightedList<EID::Nest> list;
};

struct PieceAltTrap : public Piece {
    IMPLEMENT_ME_PIECE( PieceAltTrap, PieceType::AltTrap )

    void init_new() override;

    WeightedList<EID::Trap> list;
};

struct PieceAltFurniture : public Piece {
    IMPLEMENT_ME_PIECE( PieceAltFurniture, PieceType::AltFurniture )

    void init_new() override;

    WeightedList<EID::Furn> list;
};

struct PieceAltTerrain : public Piece {
    IMPLEMENT_ME_PIECE( PieceAltTerrain, PieceType::AltTerrain )

    void init_new() override;

    WeightedList<EID::Ter> list;
};

struct PieceRemoveAll : public Piece {
    IMPLEMENT_ME_PIECE(PieceRemoveAll, PieceType::RemoveAll)
};

struct PieceUnknown : public Piece {
    IMPLEMENT_ME_PIECE(PieceUnknown, PieceType::Unknown)
};

} // namespace editor

template<>
struct enum_traits<editor::GasPumpFuel> {
    static constexpr editor::GasPumpFuel last = editor::GasPumpFuel::_Num;
};

template<>
struct enum_traits<editor::VehicleStatus> {
    static constexpr editor::VehicleStatus last = editor::VehicleStatus::_Num;
};

#endif // CATA_SRC_EDITOR_PIECE_IMPL_H
