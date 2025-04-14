#ifndef CATA_SRC_EDITOR_PIECE_IMPL_H
#define CATA_SRC_EDITOR_PIECE_IMPL_H

#include "piece.h"

#include "calendar.h"
#include "computer_enums.h"
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
    bool int_1 = true;
    bool int_2 = false;
    bool int_3 = false;
    time_duration age = 0_seconds;
    int chance = 100;
    bool remove = false;
};

struct PieceNPC : public Piece {
    IMPLEMENT_ME_PIECE( PieceNPC, PieceType::NPC )

    EID::NPCTemplate npc_class;
    bool target = false;
    std::vector<EID::Trait> traits;
    std::string unique_id;
};

struct PieceFaction : public Piece {
    IMPLEMENT_ME_PIECE( PieceFaction, PieceType::Faction )

    EID::Faction id;
};

struct PieceSign : public Piece {
    IMPLEMENT_ME_PIECE( PieceSign, PieceType::Sign )

    bool use_snippet = false;
    EID::SnippetCategory snippet;
    std::string text;
    bool use_furniture = false;
    const EID::SignageFurn default_furniture = EID::SignageFurn("f_sign");
    EID::SignageFurn furniture = default_furniture;
    // TODO: display furniture on canvas
};

struct PieceGraffiti : public Piece {
    IMPLEMENT_ME_PIECE( PieceGraffiti, PieceType::Graffiti )

    bool use_snippet = false;
    EID::SnippetCategory snippet;
    std::string text;
};

struct PieceVendingMachine : public Piece {
    IMPLEMENT_ME_PIECE( PieceVendingMachine, PieceType::VendingMachine )

    bool lootable = false;
    bool powered = false;
    bool networked = false;
    bool reinforced = false;
    bool use_default_group = true;
    EID::IGroup item_group;
    // TODO: display furniture on canvas
};

struct PieceToilet : public Piece {
    IMPLEMENT_ME_PIECE( PieceToilet, PieceType::Toilet )

    bool use_default_amount = true;
    IntRange amount;
    // TODO: display furniture on canvas
};

enum class GasPumpFuel {
    Random,
    Gasoline,
    Diesel,
    JP8,
    AvGas,

    _Num,
};

struct PieceGaspump : public Piece {
    IMPLEMENT_ME_PIECE( PieceGaspump, PieceType::GasPump )

    bool use_default_amount = true;
    IntRange amount;
    GasPumpFuel fuel;
    // TODO: display furniture on canvas
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
    const EID::Faction default_faction = EID::Faction("no_faction");
    EID::Faction faction = default_faction;
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

    void init_new() override;

    bool use_mongroup = false;
    WeightedList<EID::Monster> type_list;
    EID::MGroup group_id;
    IntRange chance = IntRange(100,100);

    // TODO: other fields
};

enum class VehicleStatus {
    LightDamage,
    Undamaged,
    Disabled,
    Pristine,

    _Num,
};

struct PieceVehicle : public Piece {
    IMPLEMENT_ME_PIECE( PieceVehicle, PieceType::Vehicle )

    EID::VGroup group_id;
    int chance = 100;
    VehicleStatus status = VehicleStatus::LightDamage;
    bool random_fuel_amount = true;
    int fuel = 100;
    std::set<int> allowed_rotations;
    const EID::Faction default_faction = EID::Faction("no_faction");
    EID::Faction faction = default_faction;

    std::unordered_set<point> silhouette() const;
};

struct PieceItem : public Piece {
    IMPLEMENT_ME_PIECE( PieceItem, PieceType::Item )

    EID::Item item_id;
    IntRange amount = IntRange(1,1);
    IntRange chance = IntRange(100,100);
};

struct PieceTrap : public Piece {
    IMPLEMENT_ME_PIECE( PieceTrap, PieceType::Trap )

    bool remove = false;
    EID::Trap id;
    // TODO: show on canvas
};

struct PieceFurniture : public Piece {
    IMPLEMENT_ME_PIECE( PieceFurniture, PieceType::Furniture )

    EID::Furn id;
    // TODO: show on canvas
};

struct PieceTerrain : public Piece {
    IMPLEMENT_ME_PIECE( PieceTerrain, PieceType::Terrain )

    EID::Ter id;
    // TODO: show on canvas
};

struct PieceTerFurnTransform : public Piece {
    IMPLEMENT_ME_PIECE( PieceTerFurnTransform, PieceType::TerFurnTransform )

    EID::TerFurnTransform id;
    // TODO: show transform result on canvas
};

struct PieceMakeRubble : public Piece {
    IMPLEMENT_ME_PIECE( PieceMakeRubble, PieceType::MakeRubble )

    bool items = false;
    bool overwrite = false;
    const EID::Furn default_rubble_type = EID::Furn("f_rubble");
    EID::Furn rubble_type = default_rubble_type;
    const EID::Ter default_floor_type = EID::Ter("t_dirt");
    EID::Ter floor_type = default_floor_type;
    // TODO: show on canvas
};

struct ComputerOption {
    std::string name;
    computer_action action = computer_action::COMPACT_NULL;
    int security = 0;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonObject& jsin);
};

struct ComputerFailure {
    computer_failure_type action = computer_failure_type::COMPFAIL_NULL;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonObject& jsin);
};

struct PieceComputer : public Piece {
    IMPLEMENT_ME_PIECE( PieceComputer, PieceType::Computer )

    std::string name;
    std::string access_denied;
    int security = 0;
    bool target = false;
    std::vector<ComputerOption> options;
    std::vector<ComputerFailure> failures;
    std::vector<std::string> chat_topics;
    std::vector<EID::EOC> eocs;
    // TODO: show on canvas
};

struct PieceSealeditem : public Piece {
    IMPLEMENT_ME_PIECE( PieceSealeditem, PieceType::SealedItem )

    // TODO
};

struct PieceZone : public Piece {
    IMPLEMENT_ME_PIECE( PieceZone, PieceType::Zone )

    EID::Zone zone_type;
    EID::Faction faction;
    std::string name;
    std::string filter;
};

struct PieceNested : public Piece {
    IMPLEMENT_ME_PIECE( PieceNested, PieceType::Nested )

    void init_new() override;

    std::unordered_set<point> silhouette() const;

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
