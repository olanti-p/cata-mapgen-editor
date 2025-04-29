#ifndef CATA_SRC_EDITOR_VIEW_CANVAS_H
#define CATA_SRC_EDITOR_VIEW_CANVAS_H

#include "point.h"
#include "common/canvas_2d.h"
#include "common/sprite_ref.h"
#include "mapgen/palette_view.h"

struct ImDrawList;

namespace editor
{
struct Camera;
struct Mapgen;
struct MapObject;
struct State;
struct SetMap;
struct UiState;
struct CanvasSnippet;

struct ViewCanvasNest {
    point pos = point::zero;
    point size = point(1, 1);
    std::string id;
};

struct ViewCanvasCell {
    MapKey key;
    ViewEntry* data = nullptr;
    SpriteRef ter;
    SpriteRef furn;
    bool has_terrain = false;
};

enum class ViewCanvasFallback {
    None,
    Predecessor,
    FillTer,
};

struct ViewCanvas {
    Mapgen& mapgen;
    ViewPalette palette;
    std::vector<ViewCanvasNest> nests;
    Canvas2D<ViewCanvasCell> matrix;
    //const CanvasSnippet* snippet = nullptr;

    std::set<SpriteRef> terrain_sprites;
    std::set<SpriteRef> furniture_sprites;

    bool show_fallbacks = true;

    ViewCanvas(State& state, Mapgen& mapgen);

    ViewCanvasFallback get_fallback_display() const;
    bool show_sprites(UiState& ui) const;
    bool show_fill_ter(UiState& ui) const;
    bool has_fill_ter() const;
    bool show_predecessor(UiState& ui) const;
    bool has_predecessor() const;
    bool has_parent() const;

    void draw_background(ImDrawList* draw_list, Camera& cam, UiState&ui) const;
    void draw_main_layer(ImDrawList* draw_list, Camera& cam, UiState& ui) const;
    void draw_overlays(ImDrawList* draw_list, Camera &cam, UiState& ui) const;
    void draw_tooltip_data_objects(Camera& cam, UiState& ui) const;
    void draw_tooltip(Camera& cam, UiState& ui) const;
    void draw_object(ImDrawList* draw_list, Camera& cam, const std::string& label, const inclusive_rectangle<point> &bb, ImColor col) const;
    void draw_hovered_outline(ImDrawList* draw_list, Camera& cam, UiState& ui) const;

    point get_tile_mouse_pos_unbounded(Camera& cam) const;
    std::optional<point> get_tile_mouse_pos_in_bounds(Camera&cam) const;
    MapKey get_tooltip_highlighted_key(Camera& cam) const;
};

/**
 * =============== Windows ===============
 */
void show_editor_view( State &state, Mapgen *mapgen_ptr );
void handle_view_change( State &state );

} // namespace editor

#endif // CATA_SRC_EDITOR_VIEW_CANVAS_H
