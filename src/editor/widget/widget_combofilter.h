// Source: https://gist.github.com/idbrii/5ddb2135ca122a0ec240ce046d9e6030
//
// Author: David Briscoe
//
// Modified ComboWithFilter with fts_fuzzy_match as include.
// Using dear imgui, v1.89 WIP
//
// Adds arrow/pgup/pgdn navigation, Enter to confirm, max_height_in_items, and
// fixed focus on open and avoids drawing past window edges.
//
// Licensed as CC0/public domain.
//
// Posted in issue: https://github.com/ocornut/imgui/issues/1658#issuecomment-1086193100

#ifndef CATA_SRC_EDITOR_WIDGET_COMBOFILTER_H
#define CATA_SRC_EDITOR_WIDGET_COMBOFILTER_H

#include <string>
#include <vector>

namespace ImGui
{
bool ComboWithFilter( const char *label, int *current_item, const std::vector<std::string> &items,
                      int popup_max_height_in_items = -1 );

} // namespace ImGui

#endif // CATA_SRC_EDITOR_WIDGET_COMBOFILTER_H
