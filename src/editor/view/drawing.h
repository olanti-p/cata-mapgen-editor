#ifndef CATA_SRC_EDITOR_DRAWING_H
#define CATA_SRC_EDITOR_DRAWING_H

#include "coordinates.h"

#include <imgui/imgui.h>

struct SpriteRef;

namespace editor
{
struct Camera;

void draw_ray(
    ImDrawList* draw_list,
    const Camera& cam,
    const point_abs_etile& start,
    const units::angle& angle_from_north,
    float length,
    ImVec4 col
);

void draw_frame(
    ImDrawList *draw_list,
    const Camera &cam,
    const point_abs_etile &p1,
    const point_abs_etile &p2,
    ImVec4 col,
    bool filled
);

void draw_frame(
    ImDrawList *draw_list,
    const Camera &cam,
    const point_abs_epos &p1,
    const point_abs_epos &p2,
    ImVec4 col,
    bool filled
);

void highlight_tile(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    ImVec4 col
);

void fill_tile(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    ImVec4 col
);

void fill_tile_sprited(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile tile,
    const SpriteRef &img,
    ImColor col
);

void highlight_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col_bg,
    ImVec4 col_border
);

void fill_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col
);

void outline_region(
    ImDrawList *draw_list,
    const Camera &cam,
    point_abs_etile p1,
    point_abs_etile p2,
    ImVec4 col
);

} // namespace editor

#endif // CATA_SRC_EDITOR_DRAWING_H
