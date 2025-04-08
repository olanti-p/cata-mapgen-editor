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
    for( const auto &it : entry.mapping.pieces ) {
        ImGui::TextDisabled( "MAP" );
        ImGui::SameLine();
        ImGui::Text( "%s", it->fmt_summary().c_str() );
    }
}

void show_palette_entry_tooltip(const Project& project, const ViewMapping& entry)
{
    for (const auto& it : entry.pieces) {
        const Palette* pal = project.get_palette(it.palette);
        if (!pal) {
            std::abort();
        }
        const PaletteEntry* e = pal->find_entry(entry.key);
        if (!e) {
            std::abort();
        }
        for (const auto& piece : e->mapping.pieces) {
            if (piece->uuid == it.id) {
                ImGui::TextDisabled("MAP");
                ImGui::SameLine();
                ImGui::Text("%s", piece->fmt_summary().c_str());
                break;
            }
        }
    }
}

} // namespace editor
