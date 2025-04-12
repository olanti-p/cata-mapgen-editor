#ifndef CATA_SRC_EDITOR_WIDGETS_H
#define CATA_SRC_EDITOR_WIDGETS_H

#include <functional>
#include <string>
#include <vector>
#include <set>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include "editable_id.h"
#include "widget_combofilter.h"
#include "common/int_range.h"
#include "common/sprite_ref.h"
#include "common/uuid.h"
#include "common/map_key.h"

#include "calendar.h"

class nc_color;
struct jmapgen_int;
class jmapgen_place;

namespace editor
{
enum class AssetType : int;
struct editor_state;
struct Palette;
} // namespace editor

namespace ImGui
{
ImVec4 curses_color_to_imgui( nc_color nc );
void SymbolColored( const std::string &sym, nc_color col );
void SymbolColored( int sym, nc_color col );
void JmapgenInt( const std::string &label, const jmapgen_int &jmi );
void JmapgenPlace( const std::string &label, const jmapgen_place &jmp );

bool InputAssetId( editor::editor_state &state, const std::string &label, std::string &buf,
                   editor::AssetType atype );

template<typename Point>
bool DragPoint( const char *label, Point *p, float v_speed = 1.0f, int v_min = 0,
                int v_max = 0, const char *format = "%d", ImGuiSliderFlags flags = 0 )
{
    int v[2];
    v[0] = p->x();
    v[1] = p->y();
    bool ret = DragScalarN( label, ImGuiDataType_S32, v, 2, v_speed, &v_min, &v_max, format, flags );
    p->x() = v[0];
    p->y() = v[1];
    return ret;
}

void Image( const SpriteRef &img, const ImVec2 &size );
bool ImageButton( const char *wid, const SpriteRef &img );
bool ImageButton( const char *wid, const SpriteRef &img, const ImVec2 &size );
bool ImageButton( const char *wid, const std::string &tile_id );
bool ImageButton( const char *wid, const std::string &tile_id, const ImVec2 &size );

/** WARNING: crashes when closing parent window! */
bool InputTextCompleting( const char *label, std::string &input,
                          const std::vector<std::string> &opts );

bool InputSymbol( const char *label, std::string &input, const char *fallback = "." );

namespace detail
{
bool InputId(
    const char *label,
    std::string &data,
    const std::vector<std::string> &opts,
    bool is_valid,
    ImGuiInputTextFlags flags,
    ImGuiInputTextCallback callback,
    void *user_data
);
}

template<typename T>
bool InputId( const char *label, editor::EditableID<T> &id, ImGuiInputTextFlags flags = 0,
              ImGuiInputTextCallback callback = nullptr, void *user_data = NULL )
{
    return detail::InputId( label, id.data, editor::EditableID<T>::get_all_opts(), id.is_valid(),
                            flags, callback, user_data );
}

bool InputIntRange( const char *label, editor::IntRange &r );

bool InputIntClamped( const char *label, int &val, int min, int max,
                      ImGuiInputTextFlags flags = 0 );

bool InputDuration( const char *label, time_duration &dur, ImGuiInputTextFlags flags = 0 );

bool VehicleDirSet( std::set<int> &data );

void TextCentered( const std::string &text );
void TextCenteredVH( const std::string &text );

bool IsDisabled();

void BeginErrorArea();
void EndErrorArea();

/**
 * Helper to display a little (?) mark which shows a tooltip when hovered.
 *
 * Copied from ImGui's demo example.
 */
void HelpMarker( const char *desc );
void HelpMarkerInline( const char *desc );
void HelpPopup( const char *desc );

void PushID(const editor::MapKey& mk);

bool PaletteSelector( const char *label, editor::UUID &current_item,
                      std::vector<editor::Palette> &options );

class VectorWidget
{
    private:
        std::function<void( size_t )> f_for_each;
        std::function<void( size_t, size_t )> f_move;
        std::function<void( size_t )> f_duplicate;
        std::function<void( size_t )> f_delete;
        std::function<bool( size_t )> f_drag_drop;
        std::function<bool()> f_add;

        std::function<bool( size_t )> f_can_duplicate;
        std::function<bool( size_t )> f_can_delete;

        bool use_default_move = false;
        bool use_default_duplicate = false;
        bool use_default_delete = false;
        bool use_default_add = false;
        bool use_explicit_move_buttons = false;
        bool use_default_drag_drop = false;

        bool run_internal( size_t num );

    public:
        VectorWidget() = default;
        ~VectorWidget() = default;

        inline VectorWidget &with_for_each( std::function<void( size_t )> &&f ) {
            f_for_each = f;
            return *this;
        }

        inline VectorWidget &with_move( std::function<void( size_t, size_t )> &&f ) {
            f_move = f;
            return *this;
        }

        inline VectorWidget &with_explicit_move_buttons() {
            use_explicit_move_buttons = true;
            return *this;
        }

        inline VectorWidget &with_default_move() {
            use_default_move = true;
            return *this;
        }

        inline VectorWidget &with_default_drag_drop() {
            use_default_drag_drop = true;
            return *this;
        }

        inline VectorWidget &with_drag_drop( std::function<bool( size_t )> &&f ) {
            f_drag_drop = f;
            return *this;
        }

        inline VectorWidget &with_can_duplicate( std::function<bool( size_t )> &&f ) {
            f_can_duplicate = f;
            return *this;
        }

        inline VectorWidget &with_duplicate( std::function<void( size_t )> &&f ) {
            f_duplicate = f;
            return *this;
        }

        inline VectorWidget &with_default_duplicate() {
            use_default_duplicate = true;
            return *this;
        }

        inline VectorWidget &with_can_delete( std::function<bool( size_t )> &&f ) {
            f_can_delete = f;
            return *this;
        }

        inline VectorWidget &with_delete( std::function<void( size_t )> &&f ) {
            f_delete = f;
            return *this;
        }

        inline VectorWidget &with_default_delete() {
            use_default_delete = true;
            return *this;
        }

        inline VectorWidget &with_add( std::function<bool()> &&f ) {
            f_add = f;
            return *this;
        }

        inline VectorWidget &with_default_add() {
            use_default_add = true;
            return *this;
        }

        template<typename T, const bool def_dupe = std::is_copy_constructible<T>::value>
        inline bool run( std::vector<T> &vec ) {
            if( !f_for_each ) {
                f_for_each = [&]( size_t idx ) {
                    ImGui::Text( "Element [%d]", static_cast<int>( idx ) );
                };
            }
            if( !f_move && use_default_move ) {
                f_move = [&]( size_t src, size_t dst ) {
                    // TODO: optimize with std::rotate
                    T elem = std::move( vec[src] );
                    vec.erase( std::next( vec.cbegin(), src ) );
                    vec.insert( std::next( vec.cbegin(), dst ), std::move( elem ) );
                };
            }
            if constexpr( def_dupe ) {
                if( !f_duplicate && use_default_duplicate ) {
                    f_duplicate = [&]( size_t idx ) {
                        vec.insert( std::next( vec.cbegin(), idx + 1 ), vec[idx] );
                    };
                }
            }
            if( !f_delete && use_default_delete ) {
                f_delete = [&]( size_t idx ) {
                    vec.erase( std::next( vec.cbegin(), idx ) );
                };
            }
            if( !f_add && use_default_add ) {
                f_add = [&]() -> bool {
                    bool ret = false;
                    if( ImGui::ImageButton( "add", "me_add" ) ) {
                        vec.emplace_back();
                        ret = true;
                    }
                    ImGui::HelpPopup( "Add new entry." );
                    return ret;
                };
            }

            return run_internal( vec.size() );
        }
};

} // namespace ImGui

#endif // CATA_SRC_EDITOR_WIDGETS_H
