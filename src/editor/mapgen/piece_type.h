#ifndef CATA_SRC_EDITOR_PIECE_TYPE_H
#define CATA_SRC_EDITOR_PIECE_TYPE_H

#include "enum_conversions.h"

namespace editor
{

enum class PieceType : int {
    Field = 0,
    NPC,
    Faction,
    Sign,
    Graffiti,
    VendingMachine,
    Toilet,
    GasPump,
    Liquid,
    IGroup,
    Loot,
    MGroup,
    Monster,
    Vehicle,
    Item,
    Trap,
    Furniture,
    Terrain,
    TerFurnTransform,
    MakeRubble,
    Computer,
    SealedItem,
    Translate,
    Zone,
    Nested,
    AltTrap,
    AltFurniture,
    AltTerrain,

    NumJmTypes
};

} // namespace editor

template<>
struct enum_traits<editor::PieceType> {
    static constexpr editor::PieceType last = editor::PieceType::NumJmTypes;
};

#endif // CATA_SRC_EDITOR_PIECE_TYPE_H
