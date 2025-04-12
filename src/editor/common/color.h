#ifndef CATA_SRC_EDITOR_COLOR_H
#define CATA_SRC_EDITOR_COLOR_H

#include <imgui/imgui.h>

namespace editor
{

const ImVec4 col_transparent = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f );
const ImVec4 col_cursor = ImVec4( 0.8f, 0.8f, 0.4f, 1.0f );
const ImVec4 col_nest = ImVec4(0.8f, 0.2f, 0.6f, 0.5f);
const ImVec4 col_vehicle_outline = ImVec4(0.2f, 0.8f, 0.6f, 0.5f);
const ImVec4 col_vehicle_dir = ImVec4(1.0f, 0.6f, 0.1f, 1.0f);
const ImVec4 col_map_object = ImVec4(1.0f, 0.6f, 0.1f, 1.0f);
const ImVec4 col_ruler = ImVec4( 0.9f, 0.9f, 0.1f, 1.0f );
const ImVec4 col_tool = ImVec4( 0.2f, 0.9f, 0.9f, 1.0f );
const ImVec4 col_sel_anim_0 = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
const ImVec4 col_sel_anim_1 = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
const ImVec4 col_sel_anim_1_snippet = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
const ImVec4 col_mapgensize_bg = ImVec4( 0.07f, 0.07f, 0.07f, 1.0f );
const ImVec4 col_default_piece_color = ImVec4( 0.07f, 0.07f, 0.07f, 1.0f );
const ImVec4 col_mapgensize_border = ImVec4( 0.7f, 0.7f, 0.7f, 1.0f );
const ImVec4 col_highlighted_palette_entry = ImVec4( 0.8f, 0.4f, 0.0f, 1.0f );
const ImVec4 col_selected_palette_entry = ImVec4( 0.9f, 0.9f, 0.1f, 1.0f );
const ImVec4 col_missing_palette_text = ImVec4(0.9f, 0.1f, 0.1f, 1.0f);
const ImVec4 col_error_text = ImVec4(0.9f, 0.1f, 0.1f, 1.0f);

} // namespace editor

#endif // CATA_SRC_EDITOR_COLOR_H
