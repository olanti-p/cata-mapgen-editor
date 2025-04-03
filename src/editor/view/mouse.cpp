#include "mouse.h"

#include "camera.h"
#include "imgui.h"

namespace editor
{

point_abs_screen get_mouse_pos()
{
    ImVec2 mouse_pos = ImGui::GetMousePos();
    return point_abs_screen( mouse_pos.x, mouse_pos.y );
}

point_abs_epos get_mouse_view_pos( const Camera &cam )
{
    point_abs_screen screen_pos = get_mouse_pos();
    return cam.screen_to_world( screen_pos );
}

point_abs_etile get_mouse_tile_pos( const Camera &cam )
{
    point_abs_screen screen_pos = get_mouse_pos();
    point_abs_epos epos = cam.screen_to_world( screen_pos );

    point_abs_etile ret;
    point_etile_epos rem;
    std::tie( ret, rem ) = project_remain<coords::etile>( epos );

    return ret;
}

} // namespace editor
