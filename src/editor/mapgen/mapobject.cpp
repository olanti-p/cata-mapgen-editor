#include "mapobject.h"

#include "mapgen.h"
#include "piece.h"
#include "project/project.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"

#include "translations.h"

namespace editor
{
MapObject::MapObject() = default;
MapObject::MapObject( MapObject && ) = default;
MapObject::~MapObject() = default;

MapObject &MapObject::operator=( MapObject && ) = default;

MapObject::MapObject( const MapObject &rhs )
{
    *this = rhs;
}

MapObject &MapObject::operator=( const MapObject &rhs )
{
    x = rhs.x;
    y = rhs.y;
    repeat = rhs.repeat;
    color = rhs.color;
    visible = rhs.visible;
    piece = rhs.piece->clone();
    return *this;
}

const UUID &MapObject::get_uuid() const
{
    return piece->uuid;
}

void MapObject::set_uuid( const UUID &uuid )
{
    piece->uuid = uuid;
}

static bool is_expanded( const State &state, const UUID &object_id )
{
    return state.ui->expanded_mapobjects.count( object_id ) != 0;
}

static void expand_object( State &state, const UUID &object_id )
{
    state.ui->expanded_mapobjects.insert( object_id );
}

static void collapse_object( State &state, const UUID &object_id )
{
    state.ui->expanded_mapobjects.erase( object_id );
}

void show_mapobjects( State &state, Mapgen &f, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 420.0f, 300.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2( 50.0f, 50.0f ), ImGuiCond_FirstUseEver );

    std::string wnd_id = string_format( "Objects list##mapobjects-%d", f.uuid );
    if( !ImGui::Begin( wnd_id.c_str(), &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( f.uuid );

    std::vector<MapObject> &list = f.objects;

    bool changed = ImGui::VectorWidget()
    .with_add( [&]() -> bool {
        std::vector<std::pair<std::string, PieceType>> object_opts;
        for( const auto &it : editor::get_piece_templates() )
        {
            PieceType pt = it->get_type();
            if( !is_available_as_mapobject( pt ) ) {
                continue;
            }
            object_opts.emplace_back( io::enum_to_string<PieceType>( pt ), pt );
        }

        std::sort( object_opts.begin(), object_opts.end(), []( const auto & a, const auto & b ) -> bool {
            return localized_compare( a, b );
        } );

        std::string new_object_str;
        new_object_str += "Add object...";
        new_object_str += '\0';
        for( const auto &it : object_opts )
        {
            new_object_str += it.first;
            new_object_str += '\0';
        }
        bool ret = false;
        int new_object_type = 0;
        if( ImGui::Combo( "##pick-new-object", &new_object_type, new_object_str.c_str() ) )
        {
            if( new_object_type != 0 ) {
                MapObject obj;
                obj.piece = editor::make_new_piece( object_opts[new_object_type - 1].second );
                UUID uuid = state.project().uuid_generator();
                obj.piece->uuid = uuid;
                obj.piece->init_new();
                list.push_back( std::move( obj ) );
                expand_object( state, uuid );
                ret = true;
            }
        }
        return ret;
    } )
    .with_for_each( [&]( size_t idx ) {
        if( list[idx].visible ) {
            if( ImGui::ImageButton( "hide", "me_visible" ) ) {
                list[idx].visible = false;
                state.mark_changed();
            }
            ImGui::HelpPopup( "Hide." );
        } else {
            if( ImGui::ImageButton( "show", "me_hidden" ) ) {
                list[idx].visible = true;
                state.mark_changed();
            }
            ImGui::HelpPopup( "Show." );
        }
        ImGui::SameLine();
        if( ImGui::ColorEdit4( "MyColor##3", ( float * )&list[idx].color,
                               ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel ) ) {
            state.mark_changed( "me-mapobject-color" );
        }
        ImGui::SameLine();
        const UUID &object_id = list[idx].get_uuid();
        if( is_expanded( state, object_id ) ) {
            if( ImGui::ArrowButton( "##collapse", ImGuiDir_Down ) ) {
                collapse_object( state, object_id );
            }
            ImGui::HelpPopup( "Hide details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), list[idx].piece->fmt_summary().c_str() );
            if( ImGui::InputIntRange( "x", list[idx].x ) ) {
                state.mark_changed( "me-mapobject-x-input" );
            }
            ImGui::SameLine();
            if( ImGui::InputIntRange( "y", list[idx].y ) ) {
                state.mark_changed( "me-mapobject-y-input" );
            }
            ImGui::SameLine();
            if( ImGui::InputIntRange( "repeat", list[idx].repeat ) ) {
                state.mark_changed( "me-mapobject-repeat-input" );
            }
            ImGui::PushID( "piece" );
            list[idx].piece->show_ui( state );
            ImGui::PopID();
            ImGui::Separator();
        } else {
            if( ImGui::ArrowButton( "##expand", ImGuiDir_Right ) ) {
                expand_object( state, object_id );
            }
            ImGui::HelpPopup( "Show details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), list[idx].piece->fmt_summary().c_str() );
        }
    } )
    .with_duplicate( [&]( size_t idx ) {
        const MapObject &src = list[ idx ];
        editor::MapObject copy = src;
        copy.set_uuid( state.project().uuid_generator() );
        list.insert( std::next( list.cbegin(), idx + 1 ), std::move( copy ) );
    } )
    .with_default_move()
    .with_default_drag_drop()
    .with_default_delete()
    .run( list );

    if( changed ) {
        state.mark_changed();
    }

    ImGui::PopID();
    ImGui::End();
}

} // namespace editor
