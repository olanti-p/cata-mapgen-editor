#include "view_canvas.h"

#include "camera.h"
#include "common/algo.h"
#include "common/canvas_2d.h"
#include "common/color.h"
#include "common/math.h"
#include "common/sprite_ref.h"
#include "common/uuid.h"
#include "coordinates.h"
#include "drawing.h"
#include "mapgen/canvas_snippet.h"
#include "mapgen/mapgen.h"
#include "mapgen/palette.h"
#include "mapgen/palette_window.h"
#include "mouse.h"
#include "project/project.h"
#include "state/control_state.h"
#include "mapgen/selection_mask.h"
#include "state/state.h"
#include "state/tools_state.h"
#include "state/ui_state.h"
#include "tool/tool.h"
#include "uistate.h"
#include "widget/widgets.h"
#include "mapgen/palette_view.h"
#include "mapgen/piece_impl.h"

#include <array>
#include <cmath>
#include <optional>
#include <set>
#include <functional>

#include <imgui/imgui.h>

namespace editor
{

static void handle_view_change_hotkey( State &state )
{
    // TODO: move this check into tool property or something
    bool is_pipette = tools::ToolKind::Pipette == state.ui->tools->get_tool();
    if( !is_pipette && state.control->has_ongoing_tool_operation() ) {
        return;
    }
    ImGuiIO &io = ImGui::GetIO();
    if( ImGui::IsKeyDown( ImGuiKey_ModAlt ) && std::abs( io.MouseWheel ) > 0.5f ) {
        int delta_wheel = static_cast<int>( std::round( io.MouseWheel ) );
        state.control->want_change_view = -delta_wheel;
    }
}

static void draw_selection_mask( ImDrawList *draw_list, const SelectionMask &selection,
                                 point_abs_etile selection_pos, const Camera &cam, bool is_snippet )
{
    if( !selection.has_selected() ) {
        return;
    }
    const auto get_mask_boundless = [&]( point p ) -> bool {
        if( selection.get_bounds().contains( p ) )
        {
            return selection.get( p );
        } else
        {
            return false;
        }
    };

    // TODO: make this not static
    static float animation_timer = 0.0f;
    animation_timer += ImGui::GetIO().DeltaTime;
    while( animation_timer > 100.0f ) {
        animation_timer -= 100.0f;
    }
    constexpr float animation_cycle_time = 1.0f;
    float cycle_stage = std::fmod( animation_timer, animation_cycle_time ) / animation_cycle_time;
    int animation_step = static_cast<int>( std::trunc( cycle_stage * 4.0f ) ) % 4;

    // TODO: smoother animation
    // TODO: less triangles
    ImVec4 c0 = col_sel_anim_0;
    ImVec4 c1 = is_snippet ? col_sel_anim_1_snippet : col_sel_anim_1;
    std::array<ImVec4, 4> colors = {
        ( animation_step == 0 || animation_step == 3 ) ? c0 : c1,
        ( animation_step == 1 || animation_step == 0 ) ? c0 : c1,
        ( animation_step == 2 || animation_step == 1 ) ? c0 : c1,
        ( animation_step == 3 || animation_step == 2 ) ? c0 : c1,
    };

    for( int y = -1; y < selection.get_size().y; y++ ) {
        for( int x = -1; x < selection.get_size().x; x++ ) {
            bool value_this = get_mask_boundless( point( x, y ) );
            bool value_right = get_mask_boundless( point( x + 1, y ) );
            bool value_below = get_mask_boundless( point( x, y + 1 ) );

            if( value_this != value_below ) {
                point_abs_etile pos_below = selection_pos + point( x, y + 1 );
                point_abs_epos p1 = coords::project_combine( pos_below, point_etile_epos( 0, -2 ) );
                point_abs_epos p2 = p1 + point_rel_epos( ETILE_SIZE / 4, 3 );
                for( int i = 0; i < 4; i++ ) {
                    int col_idx = value_this ? 3 - i : i;
                    draw_frame(
                        draw_list,
                        cam,
                        p1,
                        p2,
                        colors[ col_idx ],
                        true
                    );
                    p1.x() += ETILE_SIZE / 4;
                    p2.x() += ETILE_SIZE / 4;
                }
            }
            if( value_this != value_right ) {
                point_abs_etile pos_right = selection_pos + point( x + 1, y );
                point_abs_epos p1 = coords::project_combine( pos_right, point_etile_epos( -2, 0 ) );
                point_abs_epos p2 = p1 + point_rel_epos( 3, ETILE_SIZE / 4 );
                for( int i = 0; i < 4; i++ ) {
                    int col_idx = value_this ? i : 3 - i;
                    draw_frame(
                        draw_list,
                        cam,
                        p1,
                        p2,
                        colors[ col_idx ],
                        true
                    );
                    p1.y() += ETILE_SIZE / 4;
                    p2.y() += ETILE_SIZE / 4;
                }
            }
        }
    }
}

static MapKey get_key_at_pos(const Canvas2D<MapKey>& canvas, const editor::CanvasSnippet* snippet, point pos) {
    if (snippet)
    {
        point rel_to_snippet = pos - snippet->get_pos();
        if (snippet->get_bounds().contains(rel_to_snippet)) {
            std::optional<MapKey> data_at = snippet->get_data_at(rel_to_snippet);
            if (data_at) {
                return *data_at;
            }
        }
    }
    return canvas.get(pos);
}

Camera ViewCanvasTransform::make_camera(const Camera& cam) const
{
    Camera ret = cam;
    ret.pos -= transpose * ETILE_SIZE;
    return ret;
}

ViewCanvas::ViewCanvas(State& state, Mapgen& mapgen) : mapgen(mapgen), project(state.project()), palette(state.project()), matrix(point::zero)
{
    Palette& pal_data = *project.get_palette(mapgen.base.palette);
    palette.add_palette_recursive(pal_data, state.ui->view_palette_tree_states[pal_data.uuid]);
    palette.finalize();

    const Canvas2D<MapKey>& canvas_2d = mapgen.base.canvas;
    CanvasSnippet* snippet = state.control->snippets.get_snippet(mapgen.uuid);

    for (const MapObject& obj : mapgen.objects) {
        if (obj.visible) {
            try_add_as_nest(obj.piece.get(), point(obj.x.min, obj.y.min));
        }
    }

    SpriteRef unknown_sprite("unknown");
    if (!unknown_sprite) {
        std::abort(); // Shouldn't happen
    }

    matrix.set_size(canvas_2d.get_size());
    for (int y = 0; y < mapgen.mapgensize().y(); y++) {
        for (int x = 0; x < mapgen.mapgensize().x(); x++) {
            point_abs_etile p(x, y);
            MapKey key = get_key_at_pos(canvas_2d, snippet, p.raw());
            SpritePair img = palette.sprite_from_uuid(key);
            ViewEntry* entry = palette.find_entry(key);
            ViewCanvasCell cell;
            cell.key = key;
            cell.data = entry;
            if (entry) {
                for (const ViewPiece& it : entry->pieces) {
                    try_add_as_nest(it.piece, p.raw());
                }
            }
            if (img.furn) {
                if (!*img.furn) {
                    img.furn = &unknown_sprite;
                }
                cell.furn = *img.furn;
                furniture_sprites.insert(*img.furn);
            }
            if (img.ter) {
                if (!*img.ter) {
                    img.ter = &unknown_sprite;
                }
                cell.ter = *img.ter;
                terrain_sprites.insert(*img.ter);
                cell.has_terrain = true;
            }
            else {
                // TODO: is it correct to use sprite to test for terrain presence?
                cell.has_terrain = false;
            }
            matrix.set(p.raw(), cell);
        }
    }
}

void ViewCanvas::draw_background(ImDrawList* draw_list, Camera& cam, UiState&ui) const {
    fill_region(
        draw_list,
        cam,
        point_abs_etile(0, 0),
        point_abs_etile(-1, -1) + mapgen.mapgensize(),
        col_mapgensize_bg
    );
}

bool ViewCanvas::show_sprites(UiState& ui) const {
    return ui.show_canvas_sprites && ui.canvas_sprite_opacity > 0.01f;
}

bool ViewCanvas::show_fill_ter(UiState& ui) const {
    return mapgen.mtype == MapgenType::Oter && mapgen.oter.mapgen_base == OterMapgenFill::FillTer && ui.show_fill_ter_sprites;
}

bool ViewCanvas::has_fill_ter() const {
    return mapgen.mtype == MapgenType::Oter && mapgen.oter.mapgen_base == OterMapgenFill::FillTer && !mapgen.oter.fill_ter.is_empty() && !mapgen.oter.fill_ter.is_null();
}

bool ViewCanvas::show_predecessor(UiState& ui) const {
    return mapgen.mtype == MapgenType::Oter && mapgen.oter.mapgen_base == OterMapgenFill::PredecessorMapgen || mapgen.oter.mapgen_base == OterMapgenFill::FallbackPredecessorMapgen;
}

bool ViewCanvas::has_predecessor() const
{
    return mapgen.mtype == MapgenType::Oter && !mapgen.oter.predecessor_mapgen.is_empty() && !mapgen.oter.predecessor_mapgen.is_null();
}

bool ViewCanvas::has_parent() const
{
    return mapgen.mtype == MapgenType::Nested || mapgen.mtype == MapgenType::Update;
}

void ViewCanvas::try_add_as_nest(const Piece* piece, point pos)
{
    const PieceNested* nested = dynamic_cast<const PieceNested*>(piece);
    if (nested && !nested->preview.empty()) {
        Mapgen* mapgen = project.find_nested_mapgen_by_string(nested->preview);
        if (mapgen) {
            ViewCanvasNest nest;
            nest.pos = pos;
            nest.offset = nested->preview_pos;
            nest.mapgen = mapgen;
            nest.size = mapgen->mapgensize().raw();
            nests.push_back(nest);
        }
    }
}

ViewCanvasFallback ViewCanvas::get_fallback_display() const
{
    if (child_mode) {
        return ViewCanvasFallback::None;
    }
    if (mapgen.mtype != MapgenType::Oter || has_predecessor()) {
        return ViewCanvasFallback::Predecessor;
    }
    else {
        if (has_fill_ter()) {
            return ViewCanvasFallback::FillTer;
        }
        else {
            return ViewCanvasFallback::None;
        }
    }
}

void ViewCanvas::draw_main_layer(ImDrawList* draw_list, Camera& cam, UiState& ui) const {
    float canvas_sprite_opacity = ui.canvas_sprite_opacity;
    bool show_canvas_sprites = ui.show_canvas_sprites && canvas_sprite_opacity > 0.01f;

    if (mapgen.uses_rows()) {
        bool show_canvas_symbols = ui.show_canvas_symbols;
        SpriteRef fallback_sprite;
        ViewCanvasFallback fallback_display = get_fallback_display();

        if (show_canvas_sprites) {
            ImColor col = ImColor::ImColor(1.0f, 1.0f, 1.0f, canvas_sprite_opacity);

            if (fallback_display == ViewCanvasFallback::FillTer) {
                fallback_sprite = SpriteRef(mapgen.oter.fill_ter.data);
            }
            else if (fallback_display == ViewCanvasFallback::Predecessor) {
                fallback_sprite = SpriteRef("me_predecessor");
            }

            if (fallback_display != ViewCanvasFallback::None && fallback_sprite) {
                for (int y = 0; y < matrix.get_size().y; y++) {
                    for (int x = 0; x < matrix.get_size().x; x++) {
                        point p(x, y);
                        if (!matrix.get(p).has_terrain) {
                            fill_tile_sprited(draw_list, cam, point_abs_etile(p), fallback_sprite, col);
                        }
                    }
                }
            }

            for (const SpriteRef& ref : terrain_sprites) {
                for (int y = 0; y < matrix.get_size().y; y++) {
                    for (int x = 0; x < matrix.get_size().x; x++) {
                        point p(x, y);
                        if (matrix.get(p).ter == ref) {
                            fill_tile_sprited(draw_list, cam, point_abs_etile(p), ref, col);
                        }
                    }
                }
            }

            for (const SpriteRef& ref : furniture_sprites) {
                for (int y = 0; y < matrix.get_size().y; y++) {
                    for (int x = 0; x < matrix.get_size().x; x++) {
                        point p(x, y);
                        if (matrix.get(p).furn == ref) {
                            fill_tile_sprited(draw_list, cam, point_abs_etile(p), ref, col);
                        }
                    }
                }
            }
        }

        if (show_canvas_symbols && !child_mode) {
            for (int y = 0; y < matrix.get_size().y; y++) {
                for (int x = 0; x < matrix.get_size().x; x++) {
                    point_abs_etile p(x, y);
                    MapKey key = matrix.get(p.raw()).key;
                    std::string buf;
                    const std::string* mk;
                    bool using_fallback = false;
                    bool has_key_in_palette = palette.find_entry(key);
                    if (has_key_in_palette) {
                        buf = key.str();
                        mk = &buf;
                    }
                    else {
                        using_fallback = true;
                        if (!key) {
                            static std::string fallback = "#";
                            mk = &fallback;
                        }
                        else if (key.is_default_fill_ter_allowed() && (
                            has_fill_ter() || has_predecessor()  || has_parent()
                        ) ) {
                            buf = key.str();
                            mk = &buf;
                        }
                        else {
                            buf = "<" + key.str() + ">";
                            mk = &buf;
                        }
                    }

                    point_abs_epos center = coords::project_combine(p,
                        point_etile_epos(ETILE_SIZE / 2, ETILE_SIZE / 2));
                    point_abs_screen text_center = cam.world_to_screen(center);
                    point_rel_screen text_size(ImGui::CalcTextSize(mk->c_str()));
                    point_abs_screen text_pos = text_center - text_size.raw() / 2;
                    ImGui::SetCursorPos(text_pos.raw());
                    if (using_fallback) {
                        ImGui::PushStyleColor(ImGuiCol_Text, col_missing_palette_text);
                    }
                    ImGui::Text("%s", mk->c_str());
                    if (using_fallback) {
                        ImGui::PopStyleColor();
                    }
                }
            }
        }
    }
}

void ViewCanvas::draw_overlays(ImDrawList* draw_list, Camera& cam, UiState& ui) const {
    if (mapgen.mtype == MapgenType::Oter && mapgen.oter.matrix_mode && ui.show_omt_grid) {
        point size = mapgen.oter.om_terrain_matrix.get_size();
        constexpr point_rel_etile oter_size(SEEX * 2, SEEY * 2);
        for (int y = 0; y < size.y; y++) {
            for (int x = 0; x < size.x; x++) {
                const point_abs_etile pos_zero(oter_size.x() * x, oter_size.y() * y);
                outline_region(
                    draw_list,
                    cam,
                    pos_zero,
                    pos_zero + point_rel_etile(-1, -1) + oter_size,
                    col_mapgensize_border
                );
            }
        }
    }
    else {
        outline_region(
            draw_list,
            cam,
            point_abs_etile(0, 0),
            point_abs_etile(-1, -1) + mapgen.mapgensize(),
            col_mapgensize_border
        );
    }

    if (ui.show_canvas_objects) {
        for (const MapObject& it : mapgen.objects) {
            if (!it.visible) {
                continue;
            }
            inclusive_rectangle<point> bb = it.get_bounding_box();
            std::string label = it.piece->fmt_summary();
            draw_object(draw_list, cam, label, bb, it.color);
        }
    }

    if (ui.show_canvas_setmaps) {
        for (const SetMap& it : mapgen.setmaps) {
            if (!it.visible) {
                continue;
            }
            inclusive_rectangle<point> bb = it.get_bounding_box();
            std::string label = it.fmt_summary();
            draw_object(draw_list, cam, label, bb, it.color);
        }
    }
}

point ViewCanvas::get_tile_mouse_pos_unbounded(Camera& cam) const
{
    return get_mouse_tile_pos(cam).raw();
}

std::optional<point> ViewCanvas::get_tile_mouse_pos_in_bounds(Camera& cam) const
{
    point_abs_etile tile_pos = get_mouse_tile_pos(cam);
    point_rel_etile mapgensize = mapgen.mapgensize();
    bool is_mouse_in_bounds = tile_pos.x() >= 0 && tile_pos.y() >= 0 && tile_pos.x() < mapgensize.x() &&
        tile_pos.y() < mapgensize.y();
    if (is_mouse_in_bounds) {
        return tile_pos.raw();
    }
    else {
        return std::nullopt;
    }
}

void ViewCanvas::draw_tooltip_data_objects(Camera& cam, UiState& ui) const
{
    point mouse_tile_pos = get_tile_mouse_pos_unbounded(cam);
    if (ui.show_canvas_objects) {
        for (const MapObject& it : mapgen.objects) {
            if (!it.visible) {
                continue;
            }
            if (it.get_bounding_box().contains(mouse_tile_pos)) {
                ImGui::TextDisabled("OBJ");
                ImGui::SameLine();
                ImGui::Text("%s", it.piece->fmt_summary().c_str());
            }
        }
    }
    if (ui.show_canvas_setmaps) {
        for (const SetMap& it : mapgen.setmaps) {
            if (!it.visible) {
                continue;
            }
            if (it.get_bounding_box().contains(mouse_tile_pos)) {
                ImGui::TextDisabled("SET");
                ImGui::SameLine();
                ImGui::Text("%s", it.fmt_summary().c_str());
            }
        }
    }
}

MapKey ViewCanvas::get_tooltip_highlighted_key(Camera& cam) const
{
    std::optional<point> mouse_tile_pos = get_tile_mouse_pos_in_bounds(cam);
    if (mouse_tile_pos) {
        return matrix.get(*mouse_tile_pos).key;
    }
    return MapKey();
}

void ViewCanvas::draw_tooltip(Camera& cam, UiState& ui) const {
    std::optional<point> mouse_tile_pos = get_tile_mouse_pos_in_bounds(cam);

    if (mouse_tile_pos) {
        const ViewCanvasCell& cell = matrix.get(*mouse_tile_pos);
        const ViewEntry* entry = palette.find_entry(cell.key);

        ImGui::BeginTooltip();
        if (!cell.key) {
            ImGui::Text("ERROR: No symbol assigned here");
        }
        else if (entry) {
            show_palette_entry_tooltip(*entry);
        }
        else if (cell.key.is_default_fill_ter_allowed()) {
            if (!has_fill_ter() && !has_predecessor() && !has_parent()) {
                ImGui::Text(
                    "ERROR: No valid filler specified,\n"
                    "or terrain data not present in palette"
                );
            }
        } else {
            ImGui::Text("ERROR: Symbol not present in palette");
        }
        draw_tooltip_data_objects(cam, ui);
        // FIXME: improve logic around this check, take overwrite rules into account
        if (!cell.has_terrain) {
            if (child_mode) {
                // Show nothing
            }
            else if (has_fill_ter()) {
                ImGui::TextDisabled("FILL");
                ImGui::SameLine();
                ImGui::Text("%s", mapgen.oter.fill_ter.data.c_str());
            }
            else if (has_predecessor()) {
                ImGui::TextDisabled("PREDECESSOR");
                ImGui::SameLine();
                ImGui::Text("%s", mapgen.oter.predecessor_mapgen.data.c_str());
            }
            else if (has_parent()) {
                if (mapgen.mtype == MapgenType::Nested) {
                    ImGui::TextDisabled("PARENT");
                }
                else {
                    ImGui::TextDisabled("UNCHANGED");
                }
            }
            else {
                ImGui::Text("ERROR: No filler specified, or symbol not present in palette");
            }
        }
        ImGui::EndTooltip();
    }
    else {
        ImGui::BeginTooltip();
        ImGui::TextDisabled("Out of bounds");
        draw_tooltip_data_objects(cam, ui);
        ImGui::EndTooltip();
    }
}

void ViewCanvas::draw_object(ImDrawList* draw_list, Camera& cam, const std::string& label, const inclusive_rectangle<point>& bb, ImColor col) const
{
    point_abs_etile p1(bb.p_min);
    point_abs_etile p2(bb.p_max);
    ImVec4 col_border = col;
    ImVec4 col_text = col;
    col_text.w = 1.0f;
    ImVec4 col_bg = col;
    col_bg.w *= 0.4f;
    highlight_region(draw_list, cam, p1, p2, col_bg, col_border);
    point_abs_epos pos1 = coords::project_combine(p1, point_etile_epos(ETILE_SIZE / 2,
        ETILE_SIZE / 2));
    point_abs_epos pos2 = coords::project_combine(p2, point_etile_epos(ETILE_SIZE / 2,
        ETILE_SIZE / 2));
    point_abs_epos center((pos1.raw() + pos2.raw()) / 2);
    point_abs_screen text_center = cam.world_to_screen(center);
    point_rel_screen text_size(ImGui::CalcTextSize(label.c_str()));
    point_abs_screen text_pos = text_center - text_size.raw() / 2;
    ImGui::SetCursorPos(text_pos.raw());
    ImGui::TextColored(col_text, "%s", label.c_str());
}

void ViewCanvas::draw_hovered_outline(ImDrawList* draw_list, Camera& cam, UiState& ui) const
{
    point tile_pos = get_tile_mouse_pos_unbounded(cam);
    bool is_mouse_in_bounds = get_tile_mouse_pos_in_bounds(cam).has_value();

    std::unordered_set<point> nested_info;
    std::unordered_set<point> vehicle_info;
    std::unordered_set<int> vehicle_rotations;

    for (const MapObject& obj : mapgen.objects) {
        if (!obj.get_bounding_box().contains(tile_pos) || !obj.visible) {
            continue;
        }
        const PieceNested* nested = dynamic_cast<const PieceNested*>(obj.piece.get());
        if (nested) {
            std::unordered_set<point> sil = nested->silhouette();
            nested_info.insert(sil.begin(), sil.end());
        }
        const PieceVehicle* vehicle = dynamic_cast<const PieceVehicle*>(obj.piece.get());
        if (vehicle) {
            vehicle_rotations.insert(vehicle->allowed_rotations.begin(), vehicle->allowed_rotations.end());
            std::unordered_set<point> sil = vehicle->silhouette();
            vehicle_info.insert(sil.begin(), sil.end());
        }
    }

    if (mapgen.uses_rows() && is_mouse_in_bounds) {
        const ViewEntry* hovered_entry = matrix.get(tile_pos).data;
        if (hovered_entry) {
            for (const ViewPiece& piece : hovered_entry->pieces) {
                const PieceNested* nested = dynamic_cast<const PieceNested*>(piece.piece);
                if (nested) {
                    std::unordered_set<point> sil = nested->silhouette();
                    nested_info.insert(sil.begin(), sil.end());
                }
                const PieceVehicle* vehicle = dynamic_cast<const PieceVehicle*>(piece.piece);
                if (vehicle) {
                    vehicle_rotations.insert(vehicle->allowed_rotations.begin(), vehicle->allowed_rotations.end());
                    std::unordered_set<point> sil = vehicle->silhouette();
                    vehicle_info.insert(sil.begin(), sil.end());
                }
            }
        }
    }
    bool show_nest = false;
    if (nested_info.size() > 1) {
        show_nest = true;
    }
    else if (nested_info.size() == 1) {
        show_nest = *nested_info.begin() != point_zero;
    }
    if (show_nest) {
        for (const point& delta : nested_info) {
            fill_tile(draw_list, cam, point_abs_etile( tile_pos + delta ), col_nest);
        }
    }

    bool show_vehicle = !vehicle_info.empty();
    if (show_vehicle) {
        for (const point& delta : vehicle_info) {
            fill_tile(draw_list, cam, point_abs_etile( tile_pos + delta ), col_vehicle_outline);
        }
        for (int angle : vehicle_rotations) {
            // Reminder: 0 degrees means east, rotations are clockwise
            int angle_normalized = (angle - 90 + 360) % 360;
            draw_ray(draw_list, cam, point_abs_etile(tile_pos), units::from_degrees(angle_normalized), 1.4f, col_vehicle_dir);
        }
    }
}

void show_editor_view( State &state, Mapgen *mapgen_ptr )
{
    ImVec2 disp_size = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
    ImGui::SetNextWindowSize( disp_size );
    ImGui::Begin( "<editor_view>", nullptr,
                  ImGuiWindowFlags_NoNav |
                  ImGuiWindowFlags_NoDecoration |
                  ImGuiWindowFlags_NoFocusOnAppearing |
                  ImGuiWindowFlags_NoBackground |
                  ImGuiWindowFlags_NoBringToFrontOnFocus |
                  ImGuiWindowFlags_NoScrollWithMouse
                );

    bool view_hovered = ImGui::IsWindowHovered();

    if (ImGui::IsWindowFocused()) {
        handle_toolbar_hotkeys(state);
    }

    if( !mapgen_ptr ) {
        ImGui::BeginDisabled();
        ImGui::TextCenteredVH( "No active mapgen" );
        ImGui::EndDisabled();

        if( view_hovered ) {
            handle_view_change_hotkey( state );
        }

        ImGui::End();
        return;
    }

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    Camera &cam = *state.ui->camera;
    ViewCanvas vc(state, *mapgen_ptr);

    ImGui::PushID( vc.mapgen.uuid );
    if( view_hovered ) {
        handle_view_change_hotkey(state);
    }
    
    ImGuiIO& io = ImGui::GetIO();
    ToolsState& tools = *state.ui->tools;

    tools::ToolSettings* settings = &state.ui->tools->get_settings(tools.get_tool());
    tools::ToolHighlight tool_highlight;
    SnippetsState& snippets = state.control->snippets;

    tools::ToolTarget target{
        view_hovered,
        mapgen_ptr->uses_rows(),
        false,
        false,
        point_abs_etile(vc.get_tile_mouse_pos_unbounded(cam)),
        get_mouse_view_pos(cam),
        vc.mapgen,
        settings,
        tools.get_main_tile(),
        tool_highlight,
        vc.mapgen.get_selection_mask(),
        snippets,
    };
    tools::ToolControl& tool_control = state.control->get_tool_control(tools.get_tool());
    if (snippets.has_snippet_for(vc.mapgen.uuid) &&
        vc.mapgen.uses_rows() &&
        !tool_control.operates_on_snippets(target)) {
        CanvasSnippet snippet = snippets.drop_snippet(vc.mapgen.uuid);
        vc.mapgen.apply_snippet(snippet);
        vc.mapgen.select_from_snippet(snippet);
        state.mark_changed();
    }
    else {
        tool_control.handle_tool_operation(target);
        if (target.made_changes) {
            state.mark_changed();
        }
    }
    tools.set_main_tile(target.main_tile);

    if (!tool_control.operation_in_progress()) {
        tools.set_is_pipette_override(ImGui::IsKeyDown(ImGuiKey_ModAlt));
        if (ImGui::IsWindowFocused()) {
            handle_toolbar_hotkeys(state);
        }
    }

    if (view_hovered) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            point_rel_screen drag_delta(ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle));
            cam.drag_delta = -cam.screen_to_world(drag_delta);
        }
        else if (tools.get_tool() == tools::ToolKind::Cursor && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            // FIXME: this should be part of tool control code
            point_rel_screen drag_delta(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
            cam.drag_delta = -cam.screen_to_world(drag_delta);
        }
        else {
            cam.pos += cam.drag_delta;
            cam.drag_delta = point_rel_epos();
        }
        if (!ImGui::IsKeyDown(ImGuiKey_ModAlt) && std::abs(io.MouseWheel) > 0.5f) {
            int zoom_speed;
            if (cam.scale >= 64) {
                zoom_speed = 16;
            }
            else if (cam.scale >= 32) {
                zoom_speed = 8;
            }
            else if (cam.scale >= 16) {
                zoom_speed = 4;
            }
            else {
                zoom_speed = 2;
            }
            int delta_wheel = static_cast<int>(std::round(io.MouseWheel));
            int delta = delta_wheel * zoom_speed;
            cam.scale = clamp(cam.scale + delta, MIN_SCALE, MAX_SCALE);
        }
    }

    std::unordered_map<Mapgen*, std::unique_ptr<ViewCanvas>> nest_canvases;
    for (ViewCanvasNest& it : vc.nests) {
        if (nest_canvases.find(it.mapgen) != nest_canvases.end()) {
            continue;
        }
        ViewCanvas nest_canvas(state, *it.mapgen);
        nest_canvas.child_mode = true;
        nest_canvases.emplace( it.mapgen, std::make_unique<ViewCanvas>( std::move(nest_canvas)));
    }

    vc.draw_background(draw_list, cam, *state.ui);
    vc.draw_main_layer(draw_list, cam, *state.ui);
    for (const ViewCanvasNest&it : vc.nests) {
        ViewCanvas& canvas = *nest_canvases[it.mapgen];
        ViewCanvasTransform tf(it.pos + it.offset);
        Camera nest_cam = tf.make_camera(cam);
        canvas.draw_main_layer(draw_list, nest_cam, *state.ui);
    }
    vc.draw_overlays(draw_list, cam, *state.ui);
    for (const ViewCanvasNest& it : vc.nests) {
        ViewCanvas& canvas = *nest_canvases[it.mapgen];
        ViewCanvasTransform tf(it.pos + it.offset);
        Camera nest_cam = tf.make_camera(cam);
        canvas.draw_overlays(draw_list, nest_cam, *state.ui);
    }

    CanvasSnippet* snippet = state.control->snippets.get_snippet(vc.mapgen.uuid);
    if (snippet) {
        draw_selection_mask(draw_list, snippet->get_selection_mask(),
            point_abs_etile(snippet->get_pos()), cam, true);
    }

    SelectionMask* selection = vc.mapgen.get_selection_mask();
    if (selection) {
        draw_selection_mask(draw_list, *selection, point_abs_etile(), cam, false);
    }

    if (target.highlight.active()) {
        ImVec4 tool_color = target.highlight.color ? ImVec4(*target.highlight.color) : col_tool;
        for (const point_abs_etile& p : target.highlight.tiles) {
            ImVec4 col_bg = tool_color;
            col_bg.w *= 0.4f;
            fill_tile(draw_list, cam, p, col_bg);
            highlight_tile(draw_list, cam, p, tool_color);
        }
        for (const auto& p : target.highlight.areas) {
            auto rect = editor::normalize_rect(p.first, p.second);
            ImVec4 col_bg = tool_color;
            col_bg.w *= 0.4f;
            highlight_region(draw_list, cam, rect.first, rect.second, col_bg, tool_color);
        }
    }

    if (view_hovered) {
        vc.draw_hovered_outline(draw_list, cam, *state.ui);
        for (const ViewCanvasNest& it : vc.nests) {
            ViewCanvas& canvas = *nest_canvases[it.mapgen];
            ViewCanvasTransform tf(it.pos + it.offset);
            Camera nest_cam = tf.make_camera(cam);
            canvas.draw_hovered_outline(draw_list, nest_cam, *state.ui);
        }
        highlight_tile(draw_list, cam, get_mouse_tile_pos(cam), col_cursor);
    }

    if (target.want_tooltip) {
        ImGui::BeginTooltip();
        std::string label = tools::get_tool_definition(tools.get_tool()).get_tool_display_name();
        ImGui::SeparatorText(label.c_str());
        tool_control.show_tooltip(target);
        ImGui::EndTooltip();
    }
    if (view_hovered && ImGui::IsKeyDown(ImGuiKey_ModCtrl)) {
        if (target.want_tooltip) {
            ImGui::BeginTooltip();
            ImGui::SeparatorText("Info");
            ImGui::EndTooltip();
        }
        vc.draw_tooltip(cam, *state.ui);
        state.control->highlight_entry_in_palette = vc.get_tooltip_highlighted_key(cam);
        for (const ViewCanvasNest& it : vc.nests) {
            ViewCanvas& canvas = *nest_canvases[it.mapgen];
            ViewCanvasTransform tf(it.pos + it.offset);
            Camera nest_cam = tf.make_camera(cam);
            if (canvas.get_tile_mouse_pos_in_bounds(nest_cam)) {
                ImGui::BeginTooltip();
                ImGui::SeparatorText(canvas.mapgen.nested.imported_mapgen_id.c_str());
                ImGui::EndTooltip();
                canvas.draw_tooltip(nest_cam, *state.ui);
            }
        }
    }
    else {
        state.control->highlight_entry_in_palette = MapKey();
    }


    ImGui::PopID();
    ImGui::End();
}

void handle_view_change( State &state )
{
    const std::vector<Mapgen> &mapgens = state.project().mapgens;
    if( mapgens.empty() ) {
        return;
    }
    if( state.control->want_change_view != 0 ) {
        if( !state.ui->active_mapgen_id ) {
            state.ui->active_mapgen_id = mapgens[0].uuid;
        } else {
            int cur_idx = 0;
            for( int i = 0; i < static_cast<int>( mapgens.size() ); i++ ) {
                if( mapgens[i].uuid == state.ui->active_mapgen_id ) {
                    cur_idx = i;
                    break;
                }
            }
            int new_idx = cur_idx + state.control->want_change_view;
            new_idx = wrap_index( new_idx, state.project().mapgens.size() );
            state.ui->active_mapgen_id = mapgens[new_idx].uuid;
        }
        state.control->want_change_view = 0;
    }
}

} // namespace editor
