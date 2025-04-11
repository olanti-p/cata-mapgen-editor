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
#include "imgui.h"
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

namespace editor
{

static void handle_view_change_hotkey( State &state )
{
    if( state.control->has_ongoing_tool_operation() ) {
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
    editor::Mapgen &mapgen = *mapgen_ptr;
    ImGui::PushID( mapgen.uuid );

    fill_region(
        draw_list,
        cam,
        point_abs_etile( 0, 0 ),
        point_abs_etile( -1, -1 ) + mapgen.mapgensize(),
        col_mapgensize_bg
    );

    ImGuiIO &io = ImGui::GetIO();
    ToolsState &tools = *state.ui->tools;

    if( view_hovered ) {
        handle_view_change_hotkey(state);
    }

    ViewPalette pal(state.project());
    Palette &pal_data = *state.project().get_palette( mapgen.base.palette );
    pal.add_palette_recursive(pal_data, state.ui->view_palette_tree_states[pal_data.uuid]);
    pal.finalize();

    point_abs_etile tile_pos = get_mouse_tile_pos( cam );
    point_rel_etile mapgensize = mapgen.mapgensize();
    bool is_mouse_in_bounds = tile_pos.x() >= 0 && tile_pos.y() >= 0 && tile_pos.x() < mapgensize.x() &&
                              tile_pos.y() < mapgensize.y();
    tools::ToolSettings *settings = &state.ui->tools->get_settings( tools.get_tool() );
    tools::ToolHighlight tool_highlight;
    SelectionMask *selection = mapgen.get_selection_mask();
    SnippetsState &snippets = state.control->snippets;

    tools::ToolTarget target {
        view_hovered,
        mapgen.uses_rows(),
        false,
        false,
        tile_pos,
        get_mouse_view_pos( cam ),
        mapgen,
        settings,
        tools.get_main_tile(),
        tool_highlight,
        selection,
        snippets,
    };
    tools::ToolControl &tool_control = state.control->get_tool_control( tools.get_tool() );
    if( snippets.has_snippet_for( mapgen.uuid ) &&
        mapgen.uses_rows() &&
        !tool_control.operates_on_snippets( target ) ) {
        CanvasSnippet snippet = snippets.drop_snippet( mapgen.uuid );
        mapgen.apply_snippet( snippet );
        mapgen.select_from_snippet( snippet );
        state.mark_changed();
    } else {
        tool_control.handle_tool_operation( target );
        if( target.made_changes ) {
            state.mark_changed();
        }
    }

    bool show_tooltip = false;
    const ViewEntry *tooltip_entry = nullptr;
    MapKey tooltip_entry_uuid;
    bool tooltip_entry_error = false;
    bool tooltip_entry_fill_ter = false;
    std::string tooltip_error_msg;
    point_abs_etile tooltip_pos;

    const Canvas2D<MapKey>& canvas_2d = mapgen.base.canvas;
    const CanvasSnippet *snippet = snippets.get_snippet( mapgen.uuid );

    float canvas_sprite_opacity = state.ui->canvas_sprite_opacity;
    bool show_canvas_sprites = state.ui->show_canvas_sprites && canvas_sprite_opacity > 0.01f;
    bool show_fill_ter_fallback = show_canvas_sprites && state.ui->show_fill_ter_sprites;
    bool has_fill_ter = !mapgen.oter.fill_ter.is_empty() && !mapgen.oter.fill_ter.is_null();

    if( view_hovered ) {
        if( ImGui::IsKeyDown( ImGuiKey_ModCtrl ) ) {
            show_tooltip = true;
            tooltip_pos = tile_pos;
            if( is_mouse_in_bounds ) {
                MapKey uuid = get_key_at_pos(canvas_2d, snippet, tile_pos.raw() );
                tooltip_entry = pal.find_entry( uuid );
                tooltip_entry_uuid = uuid;
                tooltip_entry_error = !tooltip_entry;
                if (tooltip_entry_error) {
                    if (!uuid) {
                        tooltip_error_msg = "No symbol assigned here";
                    }
                    else if (has_fill_ter && uuid.is_default_fill_ter_allowed() ) {
                        // No error - silent fill_ter fallback
                        tooltip_entry_error = false;
                        tooltip_entry_fill_ter = true;
                    }
                    else {
                        tooltip_error_msg = "Symbol not present in palette";
                    }
                }
            }
        }

        if( ImGui::IsMouseDragging( ImGuiMouseButton_Right ) ) {
            point_rel_screen drag_delta( ImGui::GetMouseDragDelta( ImGuiMouseButton_Right ) );
            cam.drag_delta = -cam.screen_to_world( drag_delta );
        } else {
            cam.pos += cam.drag_delta;
            cam.drag_delta = point_rel_epos();
        }
        if( !ImGui::IsKeyDown( ImGuiKey_ModAlt ) && std::abs( io.MouseWheel ) > 0.5f ) {
            int zoom_speed;
            if( cam.scale >= 64 ) {
                zoom_speed = 16;
            } else if( cam.scale >= 32 ) {
                zoom_speed = 8;
            } else if( cam.scale >= 16 ) {
                zoom_speed = 4;
            } else {
                zoom_speed = 2;
            }
            int delta_wheel = static_cast<int>( std::round( io.MouseWheel ) );
            int delta = delta_wheel * zoom_speed;
            cam.scale = clamp( cam.scale + delta, MIN_SCALE, MAX_SCALE );
        }
        if( mapgen.uses_rows() ) {
            if( ImGui::IsMouseClicked( ImGuiMouseButton_Middle ) ) {
                if( is_mouse_in_bounds ) {
                    MapKey uuid = get_key_at_pos(canvas_2d, snippet, tile_pos.raw() );
                    tools.set_main_tile( uuid );
                } else {
                    tools.set_main_tile(MapKey());
                }
            }
        }
    }

    if( mapgen.uses_rows() ) {
        bool show_canvas_symbols = state.ui->show_canvas_symbols;
        std::optional<SpriteRef> fill_ter_fallback;

        if (show_canvas_sprites) {
            if (show_fill_ter_fallback) {
                fill_ter_fallback = SpriteRef( mapgen.oter.fill_ter.data );
            }

            // Run this for separate layers, then for each entry individually
            // to avoid draw command fragmentation from using different sprites
            if (fill_ter_fallback) {
                for (int y = 0; y < mapgen.mapgensize().y(); y++) {
                    for (int x = 0; x < mapgen.mapgensize().x(); x++) {
                        point_abs_etile p(x, y);
                        MapKey  uuid = get_key_at_pos(canvas_2d, snippet, p.raw());
                        SpritePair img = pal.sprite_from_uuid(uuid);
                        if (!img.ter) {
                            fill_tile_sprited(draw_list, cam, p, *fill_ter_fallback);
                        }
                    }
                }
            }
            for (const ViewEntry& ve : pal.entries) {
                SpritePair img = pal.sprite_from_uuid(ve.key);
                if (!img.ter) {
                    continue;
                }
                for (int y = 0; y < mapgen.mapgensize().y(); y++) {
                    for (int x = 0; x < mapgen.mapgensize().x(); x++) {
                        point_abs_etile p(x, y);
                        MapKey  uuid = get_key_at_pos(canvas_2d, snippet, p.raw());
                        if (uuid != ve.key) {
                            continue;
                        }
                        if (img.ter) {
                            fill_tile_sprited(draw_list, cam, p, *img.ter);
                        }
                    }
                }
            }
            for (const ViewEntry& ve : pal.entries) {
                SpritePair img = pal.sprite_from_uuid(ve.key);
                if (!img.furn) {
                    continue;
                }
                for (int y = 0; y < mapgen.mapgensize().y(); y++) {
                    for (int x = 0; x < mapgen.mapgensize().x(); x++) {
                        point_abs_etile p(x, y);
                        MapKey  uuid = get_key_at_pos(canvas_2d, snippet, p.raw());
                        if (uuid != ve.key) {
                            continue;
                        }
                        fill_tile_sprited(draw_list, cam, p, *img.furn);
                    }
                }
            }
        }

        for( int y = 0; y < mapgen.mapgensize().y(); y++ ) {
            for( int x = 0; x < mapgen.mapgensize().x(); x++ ) {
                point_abs_etile p( x, y );
                const MapKey &uuid = get_key_at_pos(canvas_2d, snippet, p.raw() );
                ImVec4 col = pal.color_from_uuid( uuid );
                bool has_img = false;
                if (show_canvas_sprites) {
                    has_img = fill_ter_fallback.has_value() || !pal.sprite_from_uuid(uuid).is_empty();
                }
                if( has_img ) {
                    col.w *= (1.0f - canvas_sprite_opacity);
                }
                fill_tile( draw_list, cam, p, col );
            }
        }

        if (show_canvas_symbols) {
            for (int y = 0; y < mapgen.mapgensize().y(); y++) {
                for (int x = 0; x < mapgen.mapgensize().x(); x++) {
                    point_abs_etile p(x, y);
                    MapKey uuid = get_key_at_pos(canvas_2d, snippet, p.raw());
                    std::string buf;
                    const std::string* mk;
                    bool using_fallback = false;
                    bool has_key_in_palette = pal.find_entry(uuid);
                    if (has_key_in_palette) {
                        buf = uuid.str();
                        mk = &buf;
                    } else {
                        using_fallback = true;
                        if (!uuid) {
                            static std::string fallback = "#";
                            mk = &fallback;
                        }
                        else if (has_fill_ter && uuid.is_default_fill_ter_allowed()) {
                            buf = uuid.str();
                            mk = &buf;
                        }
                        else {
                            buf = "<" + uuid.str() + ">";
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

    if( mapgen.oter.matrix_mode && state.ui->show_omt_grid ) {
        point size = mapgen.oter.om_terrain_matrix.get_size();
        constexpr point_rel_etile oter_size( SEEX * 2, SEEY * 2 );
        for( int y = 0; y < size.y; y++ ) {
            for( int x = 0; x < size.x; x++ ) {
                const point_abs_etile pos_zero( oter_size.x() * x, oter_size.y() * y );
                outline_region(
                    draw_list,
                    cam,
                    pos_zero,
                    pos_zero + point_rel_etile( -1, -1 ) + oter_size,
                    col_mapgensize_border
                );
            }
        }
    } else {
        outline_region(
            draw_list,
            cam,
            point_abs_etile( 0, 0 ),
            point_abs_etile( -1, -1 ) + mapgen.mapgensize(),
            col_mapgensize_border
        );
    }

    for( const MapObject &obj : mapgen.objects ) {
        if( !obj.visible ) {
            continue;
        }

        point_abs_etile p1( obj.x.min, obj.y.min );
        point_abs_etile p2( obj.x.max, obj.y.max );
        ImVec4 col_border = obj.color;
        ImVec4 col_text = obj.color;
        col_text.w = 1.0f;
        ImVec4 col_bg = obj.color;
        col_bg.w *= 0.4f;
        highlight_region( draw_list, cam, p1, p2, col_bg, col_border );

        std::string label = obj.piece->fmt_summary();
        point_abs_epos pos1 = coords::project_combine( p1, point_etile_epos( ETILE_SIZE / 2,
                              ETILE_SIZE / 2 ) );
        point_abs_epos pos2 = coords::project_combine( p2, point_etile_epos( ETILE_SIZE / 2,
                              ETILE_SIZE / 2 ) );
        point_abs_epos center( ( pos1.raw() + pos2.raw() ) / 2 );
        point_abs_screen text_center = cam.world_to_screen( center );
        point_rel_screen text_size( ImGui::CalcTextSize( label.c_str() ) );
        point_abs_screen text_pos = text_center - text_size.raw() / 2;
        ImGui::SetCursorPos( text_pos.raw() );
        ImGui::TextColored( col_text, "%s", label.c_str() );
    }

    if( snippet ) {
        draw_selection_mask( draw_list, snippet->get_selection_mask(),
                             point_abs_etile( snippet->get_pos() ), cam, true );
    }
    if( selection ) {
        draw_selection_mask( draw_list, *selection, point_abs_etile(), cam, false );
    }

    if( view_hovered ) {
        if (mapgen.uses_rows() && is_mouse_in_bounds) {
            MapKey  uuid = get_key_at_pos(canvas_2d, snippet, tile_pos.raw());
            ViewEntry* hovered_entry = pal.find_entry(uuid);

            std::unordered_set<point> nested_info;
            if (hovered_entry) {
                for (const ViewPiece& piece : hovered_entry->pieces) {
                    const PieceNested* nested = dynamic_cast<const PieceNested*>(piece.piece);
                    if (nested) {
                        std::unordered_set<point> sil = nested->silhouette();
                        nested_info.insert(sil.begin(), sil.end());
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
                    fill_tile(draw_list, cam, tile_pos + delta, col_nest);
                }
            }

            std::unordered_set<point> vehicle_info;
            std::unordered_set<int> vehicle_rotations;
            if (hovered_entry) {
                for (const ViewPiece& piece : hovered_entry->pieces) {
                    const PieceVehicle* vehicle = dynamic_cast<const PieceVehicle*>(piece.piece);
                    if (vehicle) {
                        vehicle_rotations.insert(vehicle->allowed_rotations.begin(), vehicle->allowed_rotations.end());
                        std::unordered_set<point> sil = vehicle->silhouette();
                        vehicle_info.insert(sil.begin(), sil.end());
                    }
                }
            }
            bool show_vehicle = !vehicle_info.empty();
            if (show_vehicle) {
                for (const point& delta : vehicle_info) {
                    fill_tile(draw_list, cam, tile_pos + delta, col_vehicle_outline);
                }
                for (int angle : vehicle_rotations) {
                    // Reminder: 0 degrees means east, rotations are clockwise
                    int angle_normalized = (angle - 90 + 360) % 360;
                    draw_ray(draw_list, cam, tile_pos, units::from_degrees(angle_normalized), 1.4f, col_vehicle_dir);
                }
            }
        }

        highlight_tile( draw_list, cam, tile_pos, col_cursor );
    }

    if( target.highlight.active() ) {
        ImVec4 tool_color = target.highlight.color ? ImVec4( *target.highlight.color ) : col_tool;
        for( const point_abs_etile &p : target.highlight.tiles ) {
            ImVec4 col_bg = tool_color;
            col_bg.w *= 0.4f;
            fill_tile( draw_list, cam, p, col_bg );
            highlight_tile( draw_list, cam, p, tool_color);
        }
        for( const auto &p : target.highlight.areas ) {
            auto rect = editor::normalize_rect( p.first, p.second );
            ImVec4 col_bg = tool_color;
            col_bg.w *= 0.4f;
            highlight_region( draw_list, cam, rect.first, rect.second, col_bg, tool_color);
        }
    }

    bool tooltip_needs_separator = false;

    if( target.want_tooltip ) {
        ImGui::BeginTooltip();
        if( tooltip_needs_separator ) {
            std::string label = tools::get_tool_definition( tools.get_tool() ).get_tool_display_name();
            ImGui::SeparatorText( label.c_str() );
        }
        tool_control.show_tooltip( target );
        ImGui::EndTooltip();
        tooltip_needs_separator = true;
    }

    state.control->highlight_entry_in_palette = tooltip_entry_uuid;

    if( show_tooltip ) {
        std::vector<const MapObject *> objects;
        for( const MapObject &obj : mapgen.objects ) {
            if( obj.x.max < tooltip_pos.x() ||
                obj.y.max < tooltip_pos.y() ||
                obj.x.min > tooltip_pos.x() ||
                obj.y.min > tooltip_pos.y()
              ) {
                continue;
            }
            objects.push_back( &obj );
        }

        if( tooltip_entry || tooltip_entry_error || (tooltip_entry_fill_ter && show_fill_ter_fallback) || !objects.empty() ) {
            ImGui::BeginTooltip();
            if( tooltip_needs_separator ) {
                ImGui::SeparatorText( "Info" );
            }
            if( tooltip_entry ) {
                show_palette_entry_tooltip( state.project(), *tooltip_entry);
            }
            if( tooltip_entry_error ) {
                std::string text = "ERROR: " + tooltip_error_msg;
                ImGui::Text( text.c_str() );
            }
            for( const MapObject *obj : objects ) {
                ImGui::TextDisabled( "OBJ" );
                ImGui::SameLine();
                ImGui::Text( "%s", obj->piece->fmt_summary().c_str() );
            }
            if (show_fill_ter_fallback && has_fill_ter && (tooltip_entry_fill_ter || !pal.sprite_from_uuid(tooltip_entry_uuid).ter)) {
                ImGui::TextDisabled("FILL");
                ImGui::SameLine();
                ImGui::Text("%s", mapgen.oter.fill_ter.data.c_str());
            }
            ImGui::EndTooltip();
            tooltip_needs_separator = true;
        }
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
