#include "ui_state_store.h"

#include "all_enum_values.h"
#include "enum_conversions.h"
#include "json.h"
#include "runtime/editor_engine.h"
#include "tool/bucket.h"
#include "tool/tool.h"
#include "tool/rectangle_tool.h"
#include "tool/selection.h"
#include "tools_state.h"
#include "ui_state.h"
#include "common/map_key.h"

#include <iostream>
#include <map>
#include <memory>

#ifdef DebugLog
#  undef DebugLog
#endif

#include <imgui/imgui_internal.h>

namespace editor
{
// TODO: Simplify this, there can only be a single instance of ui state alive in each project file.
//       It's 4 am, and it works, so I'm leaving it here as is.
static std::map<std::string, std::unique_ptr<UiState>> pdata;

UiState &get_uistate_for_project( const std::string &project_uuid )
{
    auto it = pdata.find( project_uuid );
    if( it != pdata.end() ) {
        assert( it->second );
        return *it->second;
    }
    UiState *ptr = pdata.emplace( project_uuid, std::make_unique<UiState>() ).first->second.get();
    assert( ptr );
    return *ptr;
}

void MyUserData_ClearAll( ImGuiContext *ctx, ImGuiSettingsHandler *handler )
{
    pdata.clear();
}

void MyUserData_ApplyAll( ImGuiContext *ctx, ImGuiSettingsHandler *handler )
{
    // TODO: is this needed?
}

void *MyUserData_ReadOpen( ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name )
{
    std::string project_uuid = name;
    UiState *ptr = &get_uistate_for_project( project_uuid );
    return ptr;
}

void MyUserData_ReadLine( ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry,
                          const char *line )
{
    UiState &uistate = *static_cast<UiState *>( entry );

    std::string_view line_str = line;

    if( line_str.substr( 0, 5 ) != "JSON=" ) {
        return;
    }
    std::string json_data( line_str.substr( 5 ) );
    std::istringstream ss(json_data);
    TextJsonIn jsin = TextJsonIn( ss );
    uistate.deserialize(jsin);
}

void MyUserData_WriteAll( ImGuiContext *ctx, ImGuiSettingsHandler *handler,
                          ImGuiTextBuffer *out_buf )
{
    for( const auto &it : pdata ) {
        out_buf->appendf( "[%s][%s]\n", handler->TypeName, it.first.c_str() );
        std::string data = serialize( *it.second );
        out_buf->appendf( "JSON=%s\n", data.c_str() );
        out_buf->append( "\n" );
    }
}

void initialize_settings_export()
{
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "ProjectUiState";
    ini_handler.TypeHash = ImHashStr( "ProjectUiState" );
    ini_handler.ClearAllFn = MyUserData_ClearAll;
    ini_handler.ApplyAllFn = MyUserData_ApplyAll;
    ini_handler.ReadOpenFn = MyUserData_ReadOpen;
    ini_handler.ReadLineFn = MyUserData_ReadLine;
    ini_handler.WriteAllFn = MyUserData_WriteAll;
    ImGui::AddSettingsHandler( &ini_handler );
}

} // namespace editor

namespace io
{

template<>
std::string enum_to_string<editor::tools::ToolKind>( editor::tools::ToolKind data )
{
    using editor::tools::ToolKind;
    switch( data ) {
        // *INDENT-OFF*
        case ToolKind::Brush: return "Brush";
        case ToolKind::Bucket: return "Bucket";
        case ToolKind::Cursor: return "Cursor";
        case ToolKind::Ruler: return "Ruler";
        case ToolKind::Pipette: return "Pipette";
        case ToolKind::Line: return "Line";
        case ToolKind::Rectangle: return "Rectangle";
        case ToolKind::Selection: return "Selection";
        // *INDENT-ON*
        case ToolKind::_Num:
            break;
    }
    debugmsg( "Invalid ToolKind" );
    abort();
}
} // namespace io

namespace editor
{
namespace detail
{

void OpenPalette::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "open", open );
    jsout.end_object();
}

void OpenPalette::deserialize(const TextJsonObject&jo )
{
    jo.read( "uuid", uuid );
    jo.read( "open", open );
}

void OpenMapping::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "palette", palette );
    jsout.member( "open", open );
    jsout.end_object();
}

void OpenMapping::deserialize(const TextJsonObject&jo )
{
    jo.read( "uuid", uuid );
    jo.read( "palette", palette );
    jo.read( "open", open );
}

void ExpandedPiece::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member("uuid", uuid);
    jsout.member("palette", palette);
    jsout.end_object();
}

void ExpandedPiece::deserialize(const TextJsonObject& jo)
{
    jo.read("uuid", uuid);
    jo.read("palette", palette);
}

void OpenMapgenObject::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "uuid", uuid );
    jsout.member( "open", open );
    jsout.end_object();
}

void OpenMapgenObject::deserialize(const TextJsonObject&jo )
{
    jo.read( "uuid", uuid );
    jo.read( "open", open );
}

void OpenLootDesigner::serialize(JsonOut& jsout) const
{
    jsout.start_object();
    jsout.member("is_mapping_mode", is_mapping_mode);
    jsout.member("resolved", resolved);
    jsout.member("palette", palette);
    jsout.member("map_key", map_key);
    jsout.member("mapgen", mapgen);
    jsout.member("mapgen_object", mapgen_object);
    jsout.member("open", open);
    std::vector<std::pair<UUID, bool>> val;
    val.insert(val.begin(), enabled_pieces.begin(), enabled_pieces.end());
    jsout.member("enabled_pieces", val);
    jsout.member("display_cache", display_cache);
    jsout.end_object();
}

void OpenLootDesigner::deserialize(const TextJsonObject& jo)
{
    jo.read("is_mapping_mode", is_mapping_mode);
    jo.read("resolved", resolved);
    jo.read("palette", palette);
    jo.read("map_key", map_key);
    jo.read("mapgen", mapgen);
    jo.read("mapgen_object", mapgen_object);
    jo.read("open", open);
    jo.read("display_cache", display_cache);
    std::vector<std::pair<UUID, bool>> val;
    jo.read("enabled_pieces", val);
    for (const auto& it : val) {
        enabled_pieces[it.first] = it.second;
    }
}

} // namespace detail

void UiState::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "show_demo_wnd", show_demo_wnd );
    jsout.member( "show_metrics_wnd", show_metrics_wnd );
    jsout.member( "show_project_overview", show_project_overview );
    jsout.member( "show_mapgen_info", show_mapgen_info );
    jsout.member( "show_mapgen_palette_simple", show_mapgen_palette_simple );
    jsout.member( "show_mapgen_palette_verbose", show_mapgen_palette_verbose );
    jsout.member( "show_mapgen_objects", show_mapgen_objects );
    jsout.member( "show_history", show_history );
    jsout.member( "show_camera_controls", show_camera_controls );
    jsout.member( "show_toolbar", show_toolbar );
    jsout.member( "show_autosave_params", show_autosave_params );
    jsout.member( "warn_on_import_issues", warn_on_import_issues );
    jsout.member( "show_omt_grid", show_omt_grid );
    jsout.member( "show_canvas_symbols", show_canvas_symbols );
    jsout.member( "show_canvas_sprites", show_canvas_sprites );
    jsout.member( "show_canvas_objects", show_canvas_objects );
    jsout.member( "show_canvas_setmaps", show_canvas_setmaps );
    jsout.member( "show_fill_ter_sprites", show_fill_ter_sprites );
    jsout.member( "canvas_sprite_opacity", canvas_sprite_opacity );
    jsout.member( "autosave_enabled", autosave_enabled );
    jsout.member( "autosave_interval", autosave_interval );
    jsout.member( "autosave_limit", autosave_limit );
    jsout.member( "active_mapgen_id", active_mapgen_id );
    jsout.member( "open_palette_previews", open_palette_previews );
    jsout.member( "open_source_mappings", open_source_mappings );
    jsout.member( "open_mapgenobjects", open_mapgenobjects );
    jsout.member( "open_setmaps", open_setmaps );
    jsout.member( "open_loot_designers", open_loot_designers );
    jsout.member( "camera", camera );
    jsout.member( "tools", tools );
    jsout.member( "expanded_pieces_source", expanded_pieces_source );
    jsout.member( "expanded_pieces_resolved", expanded_pieces_resolved );
    jsout.member( "expanded_mapobjects", expanded_mapobjects );
    jsout.member( "expanded_setmaps", expanded_setmaps );
    jsout.member( "project_export_path", project_export_path );
    jsout.end_object();
}

void UiState::deserialize( JSON_IN &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "show_demo_wnd", show_demo_wnd );
    jo.read( "show_metrics_wnd", show_metrics_wnd );
    jo.read( "show_project_overview", show_project_overview );
    jo.read( "show_mapgen_info", show_mapgen_info );
    jo.read( "show_mapgen_palette_simple", show_mapgen_palette_simple );
    jo.read( "show_mapgen_palette_verbose", show_mapgen_palette_verbose );
    jo.read( "show_mapgen_objects", show_mapgen_objects );
    jo.read( "show_history", show_history );
    jo.read( "show_camera_controls", show_camera_controls );
    jo.read( "show_toolbar", show_toolbar );
    jo.read( "show_autosave_params", show_autosave_params );
    jo.read( "warn_on_import_issues", warn_on_import_issues);
    jo.read( "show_omt_grid", show_omt_grid );
    jo.read( "show_canvas_symbols", show_canvas_symbols );
    jo.read( "show_canvas_sprites", show_canvas_sprites );
    jo.read( "show_canvas_objects", show_canvas_objects );
    jo.read( "show_canvas_setmaps", show_canvas_setmaps );
    jo.read( "show_fill_ter_sprites", show_fill_ter_sprites );
    jo.read( "canvas_sprite_opacity", canvas_sprite_opacity );
    jo.read( "autosave_enabled", autosave_enabled );
    jo.read( "autosave_interval", autosave_interval );
    jo.read( "autosave_limit", autosave_limit );
    jo.read( "active_mapgen_id", active_mapgen_id );
    jo.read( "open_palette_previews", open_palette_previews );
    jo.read( "open_source_mappings", open_source_mappings );
    jo.read( "open_mapgenobjects", open_mapgenobjects );
    jo.read( "open_setmaps", open_setmaps );
    jo.read( "open_loot_designers", open_loot_designers );
    jo.read( "camera", camera );
    jo.read( "tools", tools );
    jo.read( "expanded_pieces_source", expanded_pieces_source );
    jo.read( "expanded_pieces_resolved", expanded_pieces_resolved );
    jo.read( "expanded_mapobjects", expanded_mapobjects );
    jo.read( "expanded_setmaps", expanded_setmaps );
    jo.read( "project_export_path", project_export_path );
}

void Camera::serialize( JsonOut &jsout ) const
{
    // These are intentionally omitted:
    // - drag_delta
    jsout.start_object();
    jsout.member( "pos", pos );
    jsout.member( "scale", scale );
    jsout.end_object();
}

void Camera::deserialize(const TextJsonValue& jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "pos", pos );
    jo.read( "scale", scale );
}

void ToolsState::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member_as_string( "tool", tool );
    jsout.member( "brush", selected_tile );
    jsout.member( "tool_settings" );
    jsout.start_object();
    for( const auto &it : tool_settings ) {
        std::string key = io::enum_to_string( it.first );
        jsout.member( key );
        it.second->serialize( jsout );
    }
    jsout.end_object();
    jsout.end_object();
}

void ToolsState::deserialize(const TextJsonObject&jo )
{
    jo.read( "tool", tool );
    jo.read( "brush", selected_tile );
    JSON_OBJECT joset = jo.get_object( "tool_settings" );
    for( tools::ToolKind kind : all_enum_values<tools::ToolKind>() ) {
        std::string key = io::enum_to_string( kind );
        if( joset.has_object( key ) ) {
            JSON_IN *raw = joset.get_raw( key );
            tool_settings[kind] = tools::get_tool_definition( kind ).make_settings();
            tool_settings[kind]->deserialize( *raw );
        }
    }
}

} // namespace editor

namespace editor::tools
{
void ToolSettings::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.end_object();
}

void ToolSettings::deserialize( JSON_IN &jsin )
{
    JSON_OBJECT jo = jsin.get_object();
    // ...and do nothing with it
}

void BucketSettings::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "global", global );
    jsout.member( "in_selection", in_selection );
    jsout.end_object();
}

void BucketSettings::deserialize( JSON_IN &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "global", global );
    jo.read( "in_selection", in_selection );
}

void RectangleSettings::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    jsout.member( "filled", filled );
    jsout.end_object();
}

void RectangleSettings::deserialize( JSON_IN &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    jo.read( "filled", filled );
}

void SelectionSettings::serialize( JsonOut &jsout ) const
{
    jsout.start_object();
    // TODO: modes
    jsout.end_object();
}

void SelectionSettings::deserialize( JSON_IN &jsin )
{
    JSON_OBJECT jo = jsin.get_object();

    // TODO: modes
}

} // namespace editor::tools
