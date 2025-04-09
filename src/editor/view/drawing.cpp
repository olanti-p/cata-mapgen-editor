#include "drawing.h"

#include "camera.h"
#include "common/algo.h"
#include "common/canvas_2d.h"
#include "common/color.h"
#include "common/math.h"
#include "common/sprite_ref.h"
#include "imgui.h"
#include "units.h"

namespace editor
{

void draw_ray(
    ImDrawList* draw_list,
    const Camera& cam,
    const point_abs_etile& start,
    const units::angle& angle_from_north,
    float length,
    ImVec4 col
)
{
    float angle = units::to_radians(angle_from_north);
    point_abs_epos p = project_combine(start, point_etile_epos());
    ImVec2 start_raw(p.raw());
    ImVec2 start_centered = start_raw + ImVec2(ETILE_SIZE, ETILE_SIZE) / 2.0;
    ImVec2 p_a = cam.world_to_screen(start_centered);
    ImVec2 p_b = p_a + vector_rotated( ImVec2(0.0, 1.0), angle ) * cam.world_to_screen(length * ETILE_SIZE);
    draw_list->AddLine(p_a, p_b, ImColor(col), 2.0f);
}

void draw_frame(
    ImDrawList *draw_list,
    const Camera &cam,
    const point_abs_etile &p1,
    const point_abs_etile &p2,
    ImVec4 col,
    bool filled
)
{
    draw_frame(
        draw_list,
        cam,
        project_combine( p1, point_etile_epos() ),
        project_combine( p2, point_etile_epos( ETILE_SIZE - 1, ETILE_SIZE - 1 ) ),
        col,
        filled
    );
}

void draw_frame(
    ImDrawList *draw_list,
    const Camera &cam,
    const point_abs_epos &p1,
    const point_abs_epos &p2,
    ImVec4 col,
    bool filled
)
{
    ImVec2 p_min = cam.world_to_screen( p1 ).raw();
    ImVec2 p_max = cam.world_to_screen( p2 ).raw();
    if( filled ) {
        draw_list->AddRectFilled( p_min, p_max, ImColor( col ), 0.0f, ImDrawFlags_None );
    } else {
        draw_list->AddRect( p_min, p_max, ImColor( col ), 0.0f, ImDrawFlags_None, 1.0f );
    }
}

void highlight_tile(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    ImVec4 col
)
{
    draw_frame( draw_list, cam, tile, tile, col, false );
}

void fill_tile(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    ImVec4 col
)
{
    draw_frame( draw_list, cam, tile, tile, col, true );
}

void fill_tile_sprited(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    const SpriteRef &img
)
{
    ImVec2 p_min = cam.world_to_screen( project_combine( tile, point_etile_epos() ) ).raw();
    ImVec2 p_max = cam.world_to_screen( project_combine( tile, point_etile_epos( ETILE_SIZE - 1,
                                        ETILE_SIZE - 1 ) ) ).raw();
    auto uvs = img.make_uvs();
    draw_list->AddImage( img.get_tex_id(), p_min, p_max, uvs.first, uvs.second );
}

void highlight_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col_bg,
    ImVec4 col_border
)
{
    fill_region( draw_list, cam, p1, p2, col_bg );
    outline_region( draw_list, cam, p1, p2, col_border );
}

void fill_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col
)
{
    draw_frame( draw_list, cam, p1, p2, col, true );
}

void outline_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col
)
{
    draw_frame( draw_list, cam, p1, p2, col, false );
}

} // namespace editor
