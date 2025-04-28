#include "setmap.h"

#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"
#include "mapgen/mapgen.h"
#include "project/project.h"

#include "setmap_impl.h"

#define REG_SETMAP( setmap_class ) ret.push_back( std::make_unique<setmap_class>() )

namespace editor
{
SetMap::SetMap() = default;
SetMap::SetMap( SetMap && ) = default;
SetMap::~SetMap() = default;

SetMap &SetMap::operator=( SetMap && ) = default;

SetMap::SetMap( const SetMap &rhs )
{
    *this = rhs;
}

SetMap& SetMap::operator=(const SetMap& rhs)
{
    uuid = rhs.uuid;
    x = rhs.x;
    y = rhs.y;
    x2 = rhs.x2;
    y2 = rhs.y2;
    z = rhs.z;
    chance = rhs.chance;
    repeat = rhs.repeat;
    color = rhs.color;
    visible = rhs.visible;
    mode = rhs.mode;
    data = rhs.data->clone();
    return *this;
}

void SetMap::show_ui(State& state)
{
    if (ImGui::ComboEnum("mode", mode)) {
        state.mark_changed("mode");
    }

    if (ImGui::InputIntRange("x", x)) {
        state.mark_changed("x");
    }
    ImGui::SameLine();
    if (ImGui::InputIntRange("y", y)) {
        state.mark_changed("y");
    }
    ImGui::SameLine();
    if (ImGui::InputIntRange("z", z)) {
        state.mark_changed("z");
    }

    if (mode != SetMapMode::Point) {
        if (ImGui::InputIntRange("x2", x2)) {
            state.mark_changed("x2");
        }
        ImGui::SameLine();
        if (ImGui::InputIntRange("y2", y2)) {
            state.mark_changed("y2");
        }
    }

    if( ImGui::InputIntRange( "repeat", repeat ) ) {
        state.mark_changed( "repeat" );
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (ImGui::CalcTextSize("chance").x + ImGui::GetStyle().ItemSpacing.x));
    if (ImGui::InputIntClamped("chance", chance, 0, INT_MAX)) {
        state.mark_changed("chance");
    }
    ImGui::PushID("data");
    data->show_ui(state);
    ImGui::PopID();
}

std::string SetMap::fmt_summary() const
{
    return string_format(
        "%s %s: %s",
        io::enum_to_string<SetMapMode>(mode),
        io::enum_to_string<SetMapType>(data->get_type()),
        data->fmt_data_summary()
    );
}

inclusive_rectangle<point> SetMap::get_bounding_box() const
{
    int x_min = x.min;
    int x_max = x.max;
    int y_min = y.min;
    int y_max = y.max;
    if (mode != SetMapMode::Point) {
        x_min = std::min(x_min, x2.min);
        x_max = std::max(x_max, x2.max);
        y_min = std::min(y_min, y2.min);
        y_max = std::max(y_max, y2.max);
    }
    return inclusive_rectangle<point>(point(x_min, y_min), point(x_max, y_max));
}

const std::vector<std::unique_ptr<SetMapData>>& get_setmap_data_templates()
{
    static std::vector<std::unique_ptr<SetMapData>> ret;
    if (ret.empty()) {
        ret.reserve(static_cast<int>(SetMapType::_Num));
        REG_SETMAP(SetMapTer);
        REG_SETMAP(SetMapFurn);
        REG_SETMAP(SetMapTrap);
        REG_SETMAP(SetMapVariable);
        REG_SETMAP(SetMapTer);
        REG_SETMAP(SetMapBash);
        REG_SETMAP(SetMapBurn);
        REG_SETMAP(SetMapRadiation);
        REG_SETMAP(SetMapRemoveTrap);
        REG_SETMAP(SetMapRemoveCreature);
        REG_SETMAP(SetMapRemoveItem);
        REG_SETMAP(SetMapRemoveField);
    }
    return ret;
}

std::unique_ptr<SetMapData> make_new_setmap_data(SetMapType pt)
{
    for (const std::unique_ptr<SetMapData>& it : get_setmap_data_templates()) {
        if (it->get_type() == pt) {
            return it->clone();
        }
    }
    std::cerr << "Failed to generate setmap data with type " << static_cast<int>(pt) << std::endl;
    std::abort();
}

static bool is_expanded( const State &state, const UUID &object_id )
{
    return state.ui->expanded_setmaps.count( object_id ) != 0;
}

static void expand_object( State &state, const UUID &object_id )
{
    state.ui->expanded_setmaps.insert( object_id );
}

static void collapse_object( State &state, const UUID &object_id )
{
    state.ui->expanded_setmaps.erase( object_id );
}

void show_setmaps( State &state, Mapgen &f, bool is_active, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 420.0f, 300.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2( 50.0f, 50.0f ), ImGuiCond_FirstUseEver );

    std::string wnd_id;
    if (is_active) {
        wnd_id = "Active setpoints";
    }
    else {
        wnd_id = string_format("Setpoints of %s###mapobjects-%d", f.display_name(), f.uuid);
    }
    if( !ImGui::Begin( wnd_id.c_str(), &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( f.uuid );

    std::vector<SetMap> &list = f.setmaps;

    if (is_active) {
        // TODO: move this into mapgen property
        if (ImGui::ImageButton("hide", "me_hidden")) {
            for (auto& it : list) {
                it.visible = false;
            }
            state.mark_changed();
        }
        ImGui::HelpPopup("Hide all.");
        ImGui::SameLine();
        if (ImGui::ImageButton("show", "me_visible")) {
            for (auto& it : list) {
                it.visible = true;
            }
            state.mark_changed();
        }
        ImGui::HelpPopup("Show all.");
        ImGui::SameLine();
    }

    ImGui::Text("%d setmaps", list.size());

    bool changed = ImGui::VectorWidget()
    /*
    .with_add([&]() -> bool {
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
                SetMap obj;
                obj.piece = editor::make_new_piece( object_opts[new_object_type - 1].second );
                UUID uuid = state.project().uuid_generator();
                obj.piece->uuid = uuid;
                obj.piece->is_object = true;
                obj.piece->init_new();
                list.push_back( std::move( obj ) );
                expand_object( state, uuid );
                ret = true;
            }
        }
        return ret;
    } )*/
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
        const UUID& setmap_id = list[idx].uuid;
        if( is_expanded( state, setmap_id) ) {
            if( ImGui::ArrowButton( "##collapse", ImGuiDir_Down ) ) {
                collapse_object( state, setmap_id);
            }
            ImGui::HelpPopup( "Hide details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), list[idx].fmt_summary().c_str() );
            list[idx].show_ui(state);
            ImGui::Separator();
        } else {
            if( ImGui::ArrowButton( "##expand", ImGuiDir_Right ) ) {
                expand_object( state, setmap_id );
            }
            ImGui::HelpPopup( "Show details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), list[idx].fmt_summary().c_str() );
        }
    } )
    .with_duplicate( [&]( size_t idx ) {
        const SetMap &src = list[ idx ];
        editor::SetMap copy = src;
        copy.uuid = state.project().uuid_generator();
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
