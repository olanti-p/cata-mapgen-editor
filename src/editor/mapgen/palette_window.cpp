#include "palette_window.h"

#include "project/project.h"
#include "mapgen/palette.h"
#include "mapgen/palette_view.h"

#include <string>

namespace editor
{

void show_palette_entry_tooltip( const PaletteEntry &entry )
{
    std::string name = entry.display_name();
    if( !name.empty() ) {
        ImGui::Text( "%s", name.c_str() );
    }
    for( const auto &it : entry.pieces ) {
        ImGui::TextDisabled( "MAP" );
        ImGui::SameLine();
        ImGui::Text( "%s", it->fmt_summary().c_str() );
    }
}

void show_palette_entry_tooltip(const Project& project, const ViewEntry& entry)
{
    for (const auto& piece : entry.pieces) {
        ImGui::TextDisabled("MAP");
        ImGui::SameLine();
        ImGui::Text("%s", piece.piece->fmt_summary().c_str());
    }
}

} // namespace editor
