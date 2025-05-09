#include "bucket.h"

#include "common/algo.h"
#include "common/canvas_2d.h"
#include <imgui/imgui.h>
#include "mapgen/mapgen.h"

#include <vector>

namespace editor::tools
{

std::string Bucket::get_tool_display_name() const
{
    return "Bucket";
}

std::string Bucket::get_tool_hint() const
{
    return "Click LMB to fill with selected tile.";
}

ImGuiKey Bucket::get_hotkey() const
{
    return ImGuiKey_E;
}

void BucketSettings::show()
{
    ImGui::Checkbox( "Global", &global );
    ImGui::Checkbox( "In Selection", &in_selection );
}

void BucketControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.is_hovered_over_canvas() ) {
        return;
    }
    if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
        Canvas2D<MapKey> &canvas = target.mapgen.base.canvas;
        point pos = target.cursor_tile_pos.raw();
        MapKey new_value = target.main_tile;
        BucketSettings *settings = dynamic_cast<BucketSettings *>( target.settings );
        std::vector<point> affected =
            find_affected_tiles( *settings, canvas, *target.selection, pos, new_value );
        if( !affected.empty() ) {
            apply( canvas, affected, new_value );
            target.made_changes = true;
        }
    }
}

std::vector<point>
BucketControl::find_affected_tiles(
    BucketSettings &settings,
    Canvas2D<MapKey> &canvas,
    SelectionMask &selection,
    point pos,
    MapKey new_value
) const
{
    std::vector<point> ret;

    MapKey old_value = canvas.get( pos );
    if( old_value == new_value ) {
        return ret;
    }
    bool in_selection = settings.in_selection;

    const auto predicate = [ = ]( point p, const MapKey& t ) {
        if( in_selection && !selection.get( p ) ) {
            return false;
        }
        return t == old_value;
    };
    if( settings.global ) {
        ret = find_tiles_via_global( canvas, predicate );
    } else {
        ret = find_tiles_via_floodfill( canvas, pos, predicate );
    }
    return ret;
}

void BucketControl::apply( Canvas2D<MapKey> &canvas, const std::vector<point> &tiles, MapKey new_value )
{
    for( point p : tiles ) {
        canvas.set( p, new_value );
    }
}

} // namespace editor::tools
