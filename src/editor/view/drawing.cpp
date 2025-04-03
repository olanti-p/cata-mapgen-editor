#include "drawing.h"

#include "camera.h"
#include "common/canvas_2d.h"
#include "common/color.h"
#include "common/math.h"
#include "common/sprite_ref.h"
#include "imgui.h"

namespace editor
{

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
