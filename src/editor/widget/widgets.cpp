#include "widget/widgets.h"

#include "cata_tiles.h"
#include "catacharset.h"
#include "color.h"
#include "common/color.h"
#include "common/uuid.h"
#include "imgui.h"
#include "mapgen.h"
#include "mapgen/palette.h"
#include "sdl_utils.h"
#include "sdltiles.h"
#include <optional>

#ifdef DebugLog
#  undef DebugLog
#endif
#include "imgui_internal.h"

SpriteRef::SpriteRef( const std::string &id )
{
    const tileset &tset = tilecontext->get_tileset();
    const tile_type *t = tset.find_tile_type( id );
    if( t ) {
        tile_idx = t->fg.begin()->obj[0];
    }
}

std::pair<ImVec2, ImVec2> SpriteRef::make_uvs() const
{
    if( tile_idx < 0 ) {
        return std::make_pair( ImVec2( 0, 0 ), ImVec2( 1, 1 ) );
    }

    const tileset &tset = tilecontext->get_tileset();

    const texture *tex = tset.get_tile( tile_idx );

    auto rect = tex->rect();

    auto fullsize = tex->getsize();

    float w = rect.w;
    float h = rect.h;
    float x = rect.x;
    float y = rect.y;

    float fw = fullsize.x;
    float fh = fullsize.y;

    ImVec2 uv0( x / fw, y / fh );
    ImVec2 uv1( ( x + w ) / fw, ( y + h ) / fh );

    return std::make_pair( uv0, uv1 );
}

ImTextureID SpriteRef::get_tex_id() const
{
    if( tile_idx < 0 ) {
        return nullptr;
    }

    const tileset &tset = tilecontext->get_tileset();

    const texture *tex = tset.get_tile( tile_idx );

    return static_cast<void *>( tex->get_ptr() );
}

namespace ImGui
{
ImVec4 curses_color_to_imgui( nc_color nc )
{
    SDL_Color col = curses_color_to_SDL( nc );
    return ImVec4( col.r, col.g, col.b, col.a );
}

void SymbolColored( const std::string &sym, nc_color col )
{
    ImGui::TextColored( curses_color_to_imgui( col ), "%s", sym.c_str() );
}
void SymbolColored( int sym, nc_color col )
{
    SymbolColored( utf32_to_utf8( sym ), col );
}

void JmapgenInt( const std::string &label, const jmapgen_int &jmi )
{
    ImGui::Text( "%s:[%d,%d]", label.c_str(), jmi.val, jmi.valmax );
}

void JmapgenPlace( const std::string &label, const jmapgen_place &jmp )
{
    ImGui::Text( "%s", label.c_str() );
    ImGui::SameLine();
    JmapgenInt( "x", jmp.x );
    ImGui::SameLine();
    JmapgenInt( "y", jmp.y );
    ImGui::SameLine();
    JmapgenInt( "repeat", jmp.repeat );
}

bool detail::InputId( const char *label,
                      std::string &data,
                      const std::vector<std::string> &opts,
                      bool is_valid,
                      ImGuiInputTextFlags /*flags*/,
                      ImGuiInputTextCallback /*callback*/,
                      void * /*user_data*/ )
{
    // TODO: implement or remove args
    if( !is_valid ) {
        BeginErrorArea();
    }
    int current_item = -1;
    for( size_t i = 0; i < opts.size(); i++ ) {
        if( opts[i] == data ) {
            current_item = static_cast<int>( i );
            break;
        }
    }
    bool ret = ImGui::ComboWithFilter( label, &current_item, opts, 15 );
    if( current_item >= 0 ) {
        data = opts[ current_item ];
    }
    if( !is_valid ) {
        EndErrorArea();
    }
    return ret;
}

bool InputIntRange( const char *label, editor::IntRange &r )
{
    bool has_error = r.min > r.max;
    ImGui::BeginGroup();
    ImGui::Text( "%s", label );
    ImGui::SameLine();
    ImGui::PushID( label );
    if( has_error ) {
        BeginErrorArea();
    }
    ImGui::SetNextItemWidth( GetFrameHeight() * 1.5f );
    bool ret1 = ImGui::InputInt( "##min", &r.min, -1, -1, ImGuiInputTextFlags_AutoSelectAll );
    ImGui::SameLine();
    ImGui::SetNextItemWidth( GetFrameHeight() * 1.5f );
    bool ret2 = ImGui::InputInt( "##max", &r.max, -1, -1, ImGuiInputTextFlags_AutoSelectAll );
    if( has_error ) {
        EndErrorArea();
    }
    ImGui::PopID();
    ImGui::EndGroup();
    return ret1 || ret2;
}

void Image( const SpriteRef &img, const ImVec2 &size )
{
    auto uvs = img.make_uvs();
    ImGui::Image( img.get_tex_id(), size, uvs.first, uvs.second );
}

bool ImageButton( const char *wid, const SpriteRef &img )
{
    ImVec2 sz( ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() );
    return ImageButton( wid, img, sz );
}

bool ImageButton( const char *wid, const SpriteRef &img, const ImVec2 &size )
{
    auto uvs = img.make_uvs();
    bool ret = ImGui::ImageButton( wid, img.get_tex_id(), size, uvs.first, uvs.second );
    return ret;
}

bool ImageButton( const char *wid, const std::string &tile_id )
{
    return ImageButton( wid, SpriteRef( tile_id ) );
}

bool ImageButton( const char *wid, const std::string &tile_id, const ImVec2 &size )
{
    return ImageButton( wid, SpriteRef( tile_id ), size );
}

/**
 * Text input with autocomplete suggestions.
 *
 * Authors: EricStancliff & ocornut
 *
 * Source: https://github.com/ocornut/imgui/issues/718#issuecomment-1249822993
 */
bool InputTextCompleting( const char *label, std::string &input,
                          const std::vector<std::string> &opts )
{
    // Code
    const bool is_input_text_enter_pressed = ImGui::InputText( label, &input,
            ImGuiInputTextFlags_EnterReturnsTrue );
    const bool is_input_text_active = ImGui::IsItemActive();
    const bool is_input_text_activated = ImGui::IsItemActivated();

    if( is_input_text_activated ) {
        ImGui::OpenPopup( "##popup_text_completion" );
    }

    bool ret = false;

    ImGui::SetNextWindowPos( ImVec2( ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y ) );
    ImGui::SetNextWindowSize( { ImGui::GetItemRectSize().x, 0 } );
    ImGui::SetNextWindowSizeConstraints( { ImGui::GetItemRectSize().x, 0 }, { ImGui::GetItemRectSize().x, ImGui::GetFrameHeight() * 15.0f } );
    if( ImGui::BeginPopup( "##popup_text_completion",
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_ChildWindow ) ) {

        if( input.size() < 1 ) {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3, 0.3f, 0.3f, 1.0f ) );
            ImGui::Text( "> Type more" );
            ImGui::PopStyleColor();
        } else {
            bool has_matches = false;
            for( const std::string &opt : opts ) {
                if( !lcmatch( opt, input ) ) {
                    continue;
                }
                has_matches = true;
                if( ImGui::Selectable( opt.c_str() ) ) {
                    ImGui::ClearActiveID();
                    input = opt;
                }
            }
            if( !has_matches ) {
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3, 0.3f, 0.3f, 1.0f ) );
                ImGui::Text( "> Not found" );
                ImGui::PopStyleColor();
            }
        }

        if( is_input_text_enter_pressed || ( !is_input_text_active && !ImGui::IsWindowFocused() ) ) {
            ImGui::CloseCurrentPopup();
            ret = true;
        }

        ImGui::EndPopup();
    }

    return ret;
}

bool InputSymbol( const char *label, std::string &input, const char *fallback )
{
    if( input.empty() ) {
        input = fallback;
    }
    if( ImGui::InputText( label, &input, ImGuiInputTextFlags_AutoSelectAll ) ) {
        if( input.empty() ) {
            input = fallback;
        } else {
            // TODO: optionally forbid wide characters
            // TODO: accept combining characters (or remove combining chars support from map_key)
            std::u32string s32 = utf8_to_utf32( input );
            input = utf32_to_utf8( s32[0] );
        }
        return true;
    } else {
        return false;
    }
}

bool InputIntClamped( const char *label, int &val, int min, int max, ImGuiInputTextFlags flags )
{
    int val_new = val;
    if( InputInt( label, &val_new, -1, -1, flags ) ) {
        int clamped = clamp( val_new, min, max );
        if( clamped != val ) {
            val = clamped;
            return true;
        }
    }
    return false;
}

bool InputDuration( const char *label, time_duration &dur, ImGuiInputTextFlags flags )
{
    bool ret = false;
    ImGui::PushID( label );

    static bool show_decomposed = false;
    float checkbox_x_start = GetCursorPosX();
    ImGui::Checkbox( "###show-decomposed", &show_decomposed );
    ImGui::SameLine();
    float checkbox_x_size = GetCursorPosX() - checkbox_x_start;
    if( show_decomposed ) {
        int d = to_days<int>( dur );
        int h = to_hours<int>( dur % 1_days );
        int m = to_minutes<int>( dur % 1_hours );
        int s = to_seconds<int>( dur % 1_minutes );

        int d_max = to_days<int>( calendar::INDEFINITELY_LONG_DURATION ) - 1;
        int h_max = to_hours<int>( 1_days ) - 1;
        int m_max = to_minutes<int>( 1_hours ) - 1;
        int s_max = to_seconds<int>( 1_minutes ) - 1;

        float w_total = CalcItemWidth() -
                        checkbox_x_size -
                        ImGui::CalcTextSize( "d" ).x -
                        ImGui::CalcTextSize( "h" ).x -
                        ImGui::CalcTextSize( "m" ).x -
                        ImGui::CalcTextSize( "s" ).x -
                        ImGui::GetStyle().ItemSpacing.x * 7.0f;

        float w = w_total / 5.0f;

        ImGui::Text( "d" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( w * 2.0f );
        ret = ImGui::InputIntClamped( "###d", d, 0, d_max ) || ret;
        ImGui::SameLine();

        ImGui::Text( "h" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( w );
        ret = ImGui::InputIntClamped( "###h", h, 0, h_max ) || ret;
        ImGui::SameLine();

        ImGui::Text( "m" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( w );
        ret = ImGui::InputIntClamped( "###m", m, 0, m_max ) || ret;
        ImGui::SameLine();

        ImGui::Text( "s" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( w );
        ret = ImGui::InputIntClamped( "###s", s, 0, s_max ) || ret;
        ImGui::SameLine();

        ImGui::Text( "%s", label );

        dur = time_duration::from_days( d ) +
              time_duration::from_hours( h ) +
              time_duration::from_minutes( m ) +
              time_duration::from_seconds( s );
    } else {
        int t = to_turns<int>( dur );
        int t_max = to_turns<int>( calendar::INDEFINITELY_LONG_DURATION );

        float w = CalcItemWidth() -
                  checkbox_x_size -
                  ImGui::CalcTextSize( "turns" ).x -
                  ImGui::GetStyle().ItemSpacing.x;

        ImGui::Text( "turns" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( w );
        ret = ImGui::InputIntClamped( "###t", t, 0, t_max, flags );
        ImGui::SameLine();

        ImGui::Text( "%s",  label );

        dur = time_duration::from_turns( t );
    }
    ImGui::PopID();
    return ret;
}

bool VehicleDirSet( std::set<int> &data )
{
    static std::array<int, 24> button_map = {
        15 * 15,
        15 * 16,
        15 * 17,
        15 * 18,
        15 * 19,
        15 * 20,
        15 * 21,

        15 * 14,
        15 * 22,

        15 * 13,
        15 * 23,

        15 * 12,
        15 * 0,

        15 * 11,
        15 * 1,

        15 * 10,
        15 * 2,

        15 * 9,
        15 * 8,
        15 * 7,
        15 * 6,
        15 * 5,
        15 * 4,
        15 * 3,
    };

    const float btn_sz_f = ImGui::GetFrameHeight();
    ImVec2 btn_sz( btn_sz_f, btn_sz_f );
    ImVec2 btn_spacing( 2.0f, 2.0f );

    bool changed = false;

    const auto add_button = [&]( int i ) {
        int angle = button_map[ i ];
        bool is_set = data.count( angle ) != 0;

        if( is_set ) {
            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.7f, 0.7f, 0.0f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 1.0f, 1.0f, 0.2f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 1.0f, 1.0f, 0.6f, 1.0f ) );
        }

        ImGui::PushID( i );
        ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
        pos.x += btn_sz.x / 2.0f;
        pos.y += btn_sz.y / 2.0f;
        if( ImGui::Button( "##toggle_dir", btn_sz ) ) {
            if( is_set ) {
                data.erase( angle );
            } else {
                data.insert( angle );
            }
            changed = true;
        }
        ImGui::HelpPopup( string_format( "%d degrees", angle ).c_str() );
        ImGui::PopID();

        if( is_set ) {
            ImGui::PopStyleColor( 3 );
        }
    };

    ImGui::BeginGroup();
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, btn_spacing );

    float btn_pos_right;

    for( int i = 0; i < 7; i++ ) {
        if( i != 0 ) {
            ImGui::SameLine();
        }
        if( i == 6 ) {
            btn_pos_right = ImGui::GetCursorPosX();
        }
        add_button( i );
    }
    for( int i = 0; i < 5; i++ ) {
        add_button( 7 + i * 2 );

        ImGui::SameLine();
        ImGui::SetCursorPosX( btn_pos_right );

        add_button( 7 + i * 2 + 1 );
    }
    for( int i = 0; i < 7; i++ ) {
        if( i != 0 ) {
            ImGui::SameLine();
        }
        add_button( 17 + i );
    }

    ImGui::PopStyleVar();
    ImGui::EndGroup();

    return changed;
}

void TextCentered( const std::string &text )
{
    float wnd_w = ImGui::GetWindowSize().x;
    float text_w = ImGui::CalcTextSize( text.c_str() ).x;
    ImGui::SetCursorPosX( wnd_w / 2.0f - text_w / 2.0f );
    ImGui::Text( "%s", text.c_str() );
}

void TextCenteredVH( const std::string &text )
{
    ImVec2 wnd = ImGui::GetWindowSize();
    ImVec2 sz = ImGui::CalcTextSize( text.c_str() );
    ImGui::SetCursorPos( ImVec2( wnd.x / 2 - sz.x / 2, wnd.y / 2 - sz.y / 2 ) );
    ImGui::Text( "%s", text.c_str() );
}

bool IsDisabled()
{
    return ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled;
}

void BeginErrorArea()
{
    if( IsDisabled() ) {
        return;
    }
    ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.8f, 0.3f, 0.3f, 1.0f ) );
}

void EndErrorArea()
{
    if( IsDisabled() ) {
        return;
    }
    ImGui::PopStyleColor();
}

static void help_popup_common( const char *desc, ImGuiHoveredFlags flags )
{
    if( ImGui::IsItemHovered( flags ) ) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
        ImGui::TextUnformatted( desc );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void HelpMarker( const char *desc )
{
    // FIXME: find out why ? glyph renders black instead of custom color (suspecting custom fonts)
    //        After, make this render with custom color.
    ImGui::Text( "(?)" );
    help_popup_common( desc, ImGuiHoveredFlags_DelayShort );
}

void HelpMarkerInline( const char *desc )
{
    HelpMarker( desc );
    SameLine();
}

void HelpPopup( const char *desc )
{
    help_popup_common( desc, ImGuiHoveredFlags_DelayNormal );
}

void PushID(const map_key& mk)
{
    PushID(mk.str.c_str());
}

static void handle_drag_drop_source( size_t idx, const char *payload_id )
{
    if( ImGui::BeginDragDropSource() ) {
        ImGui::SetDragDropPayload( payload_id, &idx, sizeof( idx ) );
        ImGui::Text( "Drag to reorder elements." );
        ImGui::EndDragDropSource();
    }
}

static std::optional<size_t> handle_drag_drop_target( const char *payload_id )
{
    if( ImGui::BeginDragDropTarget() ) {
        if( const ImGuiPayload *payload = ImGui::AcceptDragDropPayload( payload_id ) ) {
            assert( payload->DataSize == sizeof( size_t ) );
            size_t dd = *( const size_t * )payload->Data;
            return dd;
        }
        ImGui::EndDragDropTarget();
    }
    return std::nullopt;
}

bool VectorWidget::run_internal( size_t num )
{
    std::string dd_payload_id = string_format( "vector-widget-%d", ImGui::GetID( "vector-widget" ) );
    bool dd_handled = false;

    std::optional<size_t> del;
    std::optional<size_t> dupe;
    std::optional<size_t> move_from;
    std::optional<size_t> move_to;
    for( size_t i = 0; i < num; i++ ) {
        ImGui::PushID( i );

        if( use_default_drag_drop || f_drag_drop ) {
            ImGui::ImageButton( "drag", "me_draggable" );
            if( use_default_drag_drop ) {
                handle_drag_drop_source( i, dd_payload_id.c_str() );
                std::optional<size_t> tgt = handle_drag_drop_target( dd_payload_id.c_str() );
                if( tgt ) {
                    move_from = tgt;
                    move_to = i;
                }
            } else {
                dd_handled = f_drag_drop( i ) || dd_handled;
            }
            ImGui::HelpPopup( "Drag to reorder elements." );
            ImGui::SameLine();
        }

        if( f_delete ) {
            bool disabled = f_can_delete && !f_can_delete( i );
            ImGui::BeginDisabled( disabled );
            if( ImGui::ImageButton( "del", "me_delete" ) ) {
                del = i;
            }
            ImGui::EndDisabled();
            ImGui::HelpPopup( "Delete entry." );
            ImGui::SameLine();
        }

        if( f_duplicate ) {
            bool disabled = f_can_duplicate && !f_can_duplicate( i );
            ImGui::BeginDisabled( disabled );
            if( ImGui::ImageButton( "dupe", "me_duplicate" ) ) {
                dupe = i;
            }
            ImGui::EndDisabled();
            ImGui::HelpPopup( "Duplicate entry." );
            ImGui::SameLine();
        }

        if( use_explicit_move_buttons ) {
            if( i == 0 ) {
                ImGui::BeginDisabled();
            }
            if( ImGui::ArrowButton( "up", ImGuiDir_Up ) ) {
                move_from = i;
                if( ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
                    move_to = 0;
                } else {
                    move_to = i - 1;
                }
            }
            ImGui::HelpPopup(
                "Move entry up.\n\n"
                "Hold Shift to move entry to the top."
            );
            if( i == 0 ) {
                ImGui::EndDisabled();
            }
            ImGui::SameLine();

            if( i == num - 1 ) {
                ImGui::BeginDisabled();
            }
            if( ImGui::ArrowButton( "down", ImGuiDir_Down ) ) {
                move_from = i;
                if( ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
                    move_to = num - 1;
                } else {
                    move_to = i + 1;
                }
            }
            ImGui::HelpPopup(
                "Move entry down.\n\n"
                "Hold Shift to move entry to the bottom."
            );
            if( i == num - 1 ) {
                ImGui::EndDisabled();
            }
            ImGui::SameLine();
        }

        if( f_for_each ) {
            ImGui::PushID( "elem-val" );
            f_for_each( i );
            ImGui::PopID();
        }

        ImGui::PopID();
    }
    if( use_default_drag_drop || f_drag_drop ) {
        // Invisible "end of array" drop target
        ImGui::PushStyleColor( ImGuiCol_Button, editor::col_transparent );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, editor::col_transparent );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, editor::col_transparent );
        ImGui::Button( "###end-of-array", ImVec2( -1.0, 0.0 ) );
        ImGui::PopStyleColor( 3 );
        if( use_default_drag_drop ) {
            std::optional<size_t> tgt = handle_drag_drop_target( dd_payload_id.c_str() );
            if( tgt ) {
                move_from = tgt;
                move_to = num - 1;
            }
        } else {
            dd_handled = f_drag_drop( num );
        }
    }
    bool ret = false;
    if( dd_handled ) {
        ret = true;
    }
    if( del ) {
        f_delete( *del );
        ret = true;
    }
    if( move_from && move_to && move_from != move_to ) {
        f_move( *move_from, *move_to );
        ret = true;
    }
    if( dupe ) {
        f_duplicate( *dupe );
        ret = true;
    }
    if( f_add && f_add() ) {
        ret = true;
    }
    return ret;
}

bool PaletteSelector( const char *label, editor::UUID &current_item,
                      std::vector<editor::Palette> &options )
{
    bool changed = false;
    bool is_error = true;
    std::string preview;
    for( const editor::Palette &palette : options ) {
        if( palette.uuid == current_item ) {
            is_error = false;
            preview = palette.display_name();
            break;
        }
    }
    if( is_error ) {
        ImGui::BeginErrorArea();
    }
    if( ImGui::BeginCombo( label, preview.c_str() ) ) {
        for( const editor::Palette &palette : options ) {
            ImGui::PushID( palette.uuid );
            bool is_selected = palette.uuid == current_item;
            std::string label = palette.display_name();
            if( ImGui::Selectable( label.c_str(), is_selected ) ) {
                current_item = palette.uuid;
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if( is_error ) {
        ImGui::EndErrorArea();
    }
    return changed;
}

} // namespace ImGui
