#include "palette_window.h"

#include "mapgen/palette.h"

#include <string>

namespace editor
{

void show_palette_entry_tooltip( const PaletteEntry &entry )
{
    std::string name = entry.display_name();
    if( !name.empty() ) {
        ImGui::Text( "%s", name.c_str() );
    }
    for( const auto &it : entry.mapping.pieces ) {
        ImGui::TextDisabled( "MAP" );
        ImGui::SameLine();
        ImGui::Text( "%s", it->fmt_summary().c_str() );
    }
}

} // namespace editor
