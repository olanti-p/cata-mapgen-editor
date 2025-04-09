#include "camera.h"

#include "imgui.h"

namespace editor
{
point_abs_epos Camera::screen_to_world( const point_abs_screen &p ) const
{
    point disp_size = ImGui::GetIO().DisplaySize;
    return point_abs_epos( ( p.raw() - disp_size / 2 ) * ETILE_SIZE / scale + pos.raw() +
                           drag_delta.raw() );
}

point_abs_screen Camera::world_to_screen( const point_abs_epos &p ) const
{
    point disp_size = ImGui::GetIO().DisplaySize;
    return point_abs_screen( ( p.raw() - pos.raw() - drag_delta.raw() ) * scale / ETILE_SIZE +
                             disp_size / 2 );
}

point_rel_epos Camera::screen_to_world( const point_rel_screen &p ) const
{
    return point_rel_epos( p.raw() * ETILE_SIZE / scale );
}

point_rel_screen Camera::world_to_screen( const point_rel_epos &p ) const
{
    return point_rel_screen( p.raw() * scale / ETILE_SIZE );
}

ImVec2 Camera::world_to_screen(const ImVec2& p) const
{
    point disp_size = ImGui::GetIO().DisplaySize;
    // TODO: such a mess, maybe implement matrices after all?
    return ImVec2((p - ImVec2(pos.x(), pos.y()) - ImVec2(drag_delta.x(), drag_delta.y())) * float(scale) / float(ETILE_SIZE) + ImVec2(disp_size) / 2.0f);
}

float Camera::world_to_screen(float length) const
{
    return length * scale / ETILE_SIZE;
}

} // namespace editor
