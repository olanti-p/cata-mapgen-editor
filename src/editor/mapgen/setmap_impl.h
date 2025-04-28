#ifndef CATA_SRC_EDITOR_SETMAP_IMPL_H
#define CATA_SRC_EDITOR_SETMAP_IMPL_H

#include "setmap.h"

#include "widget/editable_id.h"

namespace editor
{

struct SetMapTer : public SetMapData {
    IMPLEMENT_SETMAP( SetMapTer, SetMapType::Ter )

    EID::Ter id;
    // TODO: display all setmap on canvas
};

struct SetMapFurn : public SetMapData {
    IMPLEMENT_SETMAP(SetMapFurn, SetMapType::Furn)

    EID::Furn id;
};

struct SetMapTrap : public SetMapData {
    IMPLEMENT_SETMAP( SetMapTrap, SetMapType::Trap )

    EID::Trap id;
};

struct SetMapVariable : public SetMapData {
    IMPLEMENT_SETMAP(SetMapVariable, SetMapType::Variable)

    std::string id;
};

struct SetMapBash : public SetMapData {
    IMPLEMENT_SETMAP(SetMapBash, SetMapType::Bash)
};

struct SetMapBurn : public SetMapData {
    IMPLEMENT_SETMAP(SetMapBurn, SetMapType::Burn)
};

struct SetMapRadiation : public SetMapData {
    IMPLEMENT_SETMAP(SetMapRadiation, SetMapType::Radiation)

    IntRange amount;
};

struct SetMapRemoveTrap : public SetMapData {
    IMPLEMENT_SETMAP(SetMapRemoveTrap, SetMapType::RemoveTrap)

    EID::Trap id;
};

struct SetMapRemoveCreature : public SetMapData {
    IMPLEMENT_SETMAP(SetMapRemoveCreature, SetMapType::RemoveCreature)
};

struct SetMapRemoveItem : public SetMapData {
    IMPLEMENT_SETMAP(SetMapRemoveItem, SetMapType::RemoveItem)
};

struct SetMapRemoveField : public SetMapData {
    IMPLEMENT_SETMAP(SetMapRemoveField, SetMapType::RemoveField)
};

} // namespace editor

#endif // CATA_SRC_EDITOR_SETMAP_IMPL_H
