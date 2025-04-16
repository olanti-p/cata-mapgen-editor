#ifndef CATA_SRC_EDITOR_LOOT_DESIGNER_H
#define CATA_SRC_EDITOR_LOOT_DESIGNER_H

namespace editor
{
struct State;

namespace detail {
struct OpenLootDesigner;
} // namespace detail

void show_loot_designer(State& state, detail::OpenLootDesigner& instance);

} // namespace editor

#endif // CATA_SRC_EDITOR_LOOT_DESIGNER_H
