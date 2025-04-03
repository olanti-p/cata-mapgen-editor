#include "palette.h"

#include "imgui.h"
#include "common/color.h"
#include "project/project.h"
#include "state/control_state.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"
#include "mapgen/palette_view.h"

#include <algorithm>
#include <vector>

namespace editor
{
static int find_dragged_idx( const Palette &palette, UUID uuid )
{
    for( size_t i = 0; i < palette.entries.size(); i++ ) {
        if( palette.entries[i].uuid == uuid ) {
            return i;
        }
    }
    return -1;
}

bool handle_palette_entry_drag_and_drop( Project &project, Palette &palette, int idx )
{
    bool ret = false;

    const char *payload_id = "PALETTE_ENTRY";

    std::vector<PaletteEntry> &entries = palette.entries;
    const int num_entries = static_cast<int>( entries.size() );
    bool is_last_entry = idx == num_entries;
    if( !is_last_entry ) {
        if( ImGui::BeginDragDropSource() ) {
            PaletteEntryDragState dd;
            dd.palette = palette.uuid;
            dd.entry = entries[idx].uuid;
            ImGui::SetDragDropPayload( payload_id, &dd, sizeof( dd ) );
            ImGui::Text( "Drag to reorder elements." );
            ImGui::EndDragDropSource();
        }
    }
    if( ImGui::BeginDragDropTarget() ) {
        if( const ImGuiPayload *payload = ImGui::AcceptDragDropPayload( payload_id ) ) {
            assert( payload->DataSize == sizeof( PaletteEntryDragState ) );
            PaletteEntryDragState dd = *( const PaletteEntryDragState * )payload->Data;

            Palette &source_palette = *project.get_palette( dd.palette );
            int dragged_idx = find_dragged_idx( source_palette, dd.entry );

            if( &source_palette != &palette ) {
                // Dragging between different palettes

                PaletteEntry entry = std::move( source_palette.entries[dragged_idx] );
                source_palette.entries.erase( source_palette.entries.begin() + dragged_idx );

                if( is_last_entry ) {
                    entries.emplace_back( std::move( entry ) );
                } else {
                    entries.insert( entries.begin() + idx, std::move( entry ) );
                }
                ret = true;
            } else if( dragged_idx != idx && ( !is_last_entry || dragged_idx != ( num_entries - 1 ) ) ) {
                // We don't want to react to the element being dragged onto itself.
                // We don't want to react to the last element being dragged to the end.

                PaletteEntry entry = std::move( entries[dragged_idx] );
                entries.erase( entries.begin() + dragged_idx );
                if( is_last_entry ) {
                    entries.emplace_back( std::move( entry ) );
                } else {
                    entries.insert( entries.begin() + idx, std::move( entry ) );
                }
                ret = true;
            }
        }
        ImGui::EndDragDropTarget();
    }

    return ret;
}

static void show_palette_entries_simple( State &state, Palette &palette )
{
    UUID selected = state.ui->tools->get_main_tile();
    ImGuiStyle &style = ImGui::GetStyle();
    int buttons_count = palette.entries.size();
    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 button_sz( 40, 40 );
    ImVec2 button_sz_text = button_sz + ImGui::GetStyle().FramePadding * 2;
    for( int idx = 0; idx <= buttons_count; idx++ ) {
        if( idx == buttons_count ) {
            // Pseudo button, to allow dropping elements at the end
            ImGui::PushStyleColor( ImGuiCol_Button, col_transparent );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, col_transparent );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, col_transparent );
            ImGui::Button( "###drop-target", button_sz_text );
            ImGui::PopStyleColor( 3 );
            if( handle_palette_entry_drag_and_drop( state.project(), palette, idx ) ) {
                state.mark_changed();
            }
            continue;
        }
        const PaletteEntry &entry = palette.entries[idx];
        const SpriteRef *img = palette.sprite_from_uuid( entry.uuid );
        ImGui::PushID( idx );
        bool is_selected = selected == entry.uuid;
        bool is_highlighted = state.control->highlight_entry_in_palette == entry.uuid;
        if( is_highlighted ) {
            ImGui::PushStyleColor( ImGuiCol_Button, col_highlighted_palette_entry );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, col_highlighted_palette_entry );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, col_highlighted_palette_entry );
        } else if( is_selected ) {
            ImGui::PushStyleColor( ImGuiCol_Button, col_selected_palette_entry );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, col_selected_palette_entry );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, col_selected_palette_entry );
        }
        bool btn_result;
        if( img ) {
            btn_result = ImGui::ImageButton( "button", *img, button_sz );
        } else {
            std::string label = string_format( "%s###button", entry.key.str );
            btn_result = ImGui::Button( label.c_str(), button_sz_text );
        }
        if( is_selected || is_highlighted ) {
            ImGui::PopStyleColor( 3 );
        }
        if( handle_palette_entry_drag_and_drop( state.project(), palette, idx ) ) {
            state.mark_changed();
        }
        if( btn_result && !is_selected ) {
            state.ui->tools->set_main_tile( entry.uuid );
        }
        if( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayShort ) ) {
            ImGui::BeginTooltip();
            show_palette_entry_tooltip( entry );
            ImGui::EndTooltip();
        }
        float last_button_x2 = ImGui::GetItemRectMax().x;
        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x;
        if( idx + 1 <= buttons_count && next_button_x2 < window_visible_x2 ) {
            ImGui::SameLine();
        }
        ImGui::PopID();
    }
}

void show_palette_simple( State &state, Palette &p, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 670.0f, 120.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2( 50.0f, 50.0f ), ImGuiCond_FirstUseEver );

    std::string name = p.display_name();
    std::string wnd_id = string_format( "Palette %s###palette-%d-simple", name, p.uuid );
    if( !ImGui::Begin( wnd_id.c_str(), &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( p.uuid );
    ImGui::Text( "%s", name.c_str() );
    if( ImGui::Button( "Toggle verbose mode" ) ) {
        state.ui->toggle_show_palette_verbose( p.uuid );
    }

    show_palette_entries_simple( state, p );

    ImGui::PopID();
    ImGui::End();
}

} // namespace editor
