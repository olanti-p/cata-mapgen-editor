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

enum class cube_direction : int;
enum class direction : unsigned;
enum class ot_match_type : int;
enum class oter_flags : int;

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

    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;

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

    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;

    bool use_default_amount = true;
    IntRange amount;
    EID::Liquid liquid;
    bool spawn_always = true;
    IntRange chance;
};

struct PieceIGroup : public Piece {
    IMPLEMENT_ME_PIECE( PieceIGroup, PieceType::IGroup )

    IntRange get_repeat() const override { return repeat; };
    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;

    EID::IGroup group_id;
    IntRange chance;
    bool spawn_once = true;
    IntRange repeat = IntRange(1,1);
    const EID::Faction default_faction = EID::Faction("no_faction");
    EID::Faction faction = default_faction;
};

struct PieceLoot : public Piece {
    IMPLEMENT_ME_PIECE( PieceLoot, PieceType::Loot )

    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;
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

    IntRange get_repeat() const override { return repeat; };
    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;

    EID::Item item_id;
    std::string variant;
    IntRange amount = IntRange(1,1);
    IntRange chance = IntRange(100,100);
    IntRange repeat = IntRange(1, 1);
    std::vector<EID::Flag> custom_flags;
    const EID::Faction default_faction = EID::Faction("no_faction");
    EID::Faction faction = default_faction;
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

    void roll_loot(std::vector<item>& result, time_point turn, float spawnrate) const override;
    // TODO
};

struct PieceZone : public Piece {
    IMPLEMENT_ME_PIECE( PieceZone, PieceType::Zone )

    EID::Zone zone_type;
    EID::Faction faction;
    std::string name;
    std::string filter;
};

enum NestedCheckType {
    Oter,
    Join,
    Flag,
    FlagAny,
    Predecessor,
    ZLevel,

    _Num,
};

struct NestedCheck {
    NestedCheck() = default;
    virtual ~NestedCheck() = default;

    static std::unique_ptr<NestedCheck> make(NestedCheckType type);

    virtual NestedCheckType get_type() const = 0;
    virtual std::unique_ptr<NestedCheck> clone() const = 0;
    virtual void serialize(JsonOut& jsout) const = 0;
    virtual void deserialize(const TextJsonObject& jsin) = 0;
    virtual void show_ui(State& state) = 0;
};

struct NestedCheckOter : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckOter, NestedCheckType::Oter)

    bool operator<(const NestedCheckOter& rhs) const {
        if (dir < rhs.dir) {
            return true;
        }
        return matches < rhs.matches;
    }

    direction dir;
    std::vector<std::pair<std::string, ot_match_type>> matches;
};

struct NestedCheckJoin : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckJoin, NestedCheckType::Join)

    bool operator<(const NestedCheckJoin& rhs) const {
        if (dir < rhs.dir) {
            return true;
        }
        return matches < rhs.matches;
    }

    cube_direction dir;
    std::vector<std::string> matches;
};

struct NestedCheckFlag : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckFlag, NestedCheckType::Flag)

    bool operator<(const NestedCheckFlag& rhs) const {
        if (dir < rhs.dir) {
            return true;
        }
        return matches < rhs.matches;
    }

    direction dir;
    std::vector<oter_flags> matches;
};

struct NestedCheckFlagAny : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckFlagAny, NestedCheckType::FlagAny)

    bool operator<(const NestedCheckFlagAny& rhs) const {
        if (dir < rhs.dir) {
            return true;
        }
        return matches < rhs.matches;
    }

    direction dir;
    std::vector<oter_flags> matches;
};

struct NestedCheckPredecessor : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckPredecessor, NestedCheckType::Predecessor)

    bool operator<(const NestedCheckPredecessor& rhs) const {
        if (match_terrain < rhs.match_terrain) {
            return true;
        }
        return match_type < rhs.match_type;
    }

    std::string match_terrain;
    ot_match_type match_type;
};

struct NestedCheckZLevel : public NestedCheck {
    IMPLEMENT_NESTED_CHECK(NestedCheckZLevel, NestedCheckType::ZLevel)

    bool operator<(const NestedCheckZLevel& rhs) const {
        return z < rhs.z;
    }

    int z = 0;
};

struct PieceNested : public Piece {
    IMPLEMENT_ME_PIECE_NOCOPY( PieceNested, PieceType::Nested )

    PieceNested() = default;
    PieceNested(const PieceNested& rhs);

    void init_new() override;

    std::unordered_set<point> silhouette() const;

    WeightedList<EID::Nest> chunks;
    WeightedList<EID::Nest> else_chunks;
    std::vector<std::unique_ptr<NestedCheck>> checks;
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

template<>
struct enum_traits<editor::NestedCheckType> {
    static constexpr editor::NestedCheckType last = editor::NestedCheckType::_Num;
};

#endif // CATA_SRC_EDITOR_PIECE_IMPL_H
