#include "selection.h"

#include "common/algo.h"
#include "coordinates.h"
#include "imgui.h"
#include "line.h"
#include "mapgen/canvas_snippet.h"
#include "mapgen/mapgen.h"
#include "state/control_state.h"
#include "point.h"

#include <vector>

namespace editor::tools
{

std::string Selection::get_tool_display_name() const
{
    return "Select";
}

std::string Selection::get_tool_hint() const
{
    return  "Multi-purpose selection and manipulation tool.\n"
            "\n"
            "Drag LMB to select in a rectangular shape.\n"
            "Drag selected area with LMB to move selection.\n"
            "Hold Shift to add to existing selection.\n"
            "Press Esc while dragging to cancel selection.\n"
            "Press Esc or click outside selected area to dismiss selection.\n"
            "Press Delete to erase selected area.\n"
            "Press Ctrl+A to select everything.\n"
            "Press Ctrl+C to copy selected area to clipboard.\n"
            "Press Ctrl+X to cut selected area to clipboard.\n"
            "Press Ctrl+V to paste from clipboard.\n";
}

void SelectionSettings::show()
{
    // TODO: selection modes
}

static std::optional<CanvasSnippet> try_import_snippet_from_clipboard() {
    // TODO: ensure this matches mapgen parsing logic
    // TODO: check for backslashes, double-width and combining characters
    std::string text(ImGui::GetClipboardText());
    if (text.empty()) {
        return std::nullopt;
    }
    std::vector<std::string> lines = string_split(text, '\n');
    std::vector<std::vector<std::string_view>> matrix;
    size_t size_y = lines.size();
    size_t size_x = 0;
    for (const std::string& line : lines) {
        std::vector<std::string_view> symbols;
        utf8_display_split_into(line, symbols);
        size_x = std::max(size_x, symbols.size());
        matrix.emplace_back(std::move(symbols));
    }
    if (size_x > (24 * 4) || size_y > (24 * 4)) {
        // Arbitrary limit, should cover most use cases
        return std::nullopt;
    }
    point size(size_x, size_y);
    Canvas2D<map_key> data(size, map_key());
    Canvas2D<Bool> mask(size, Bool(false));
    for (size_t y = 0; y < matrix.size(); y++) {
        const std::vector<std::string_view>& row = matrix[y];
        for (size_t x = 0; x < row.size(); x++) {
            point pos(x, y);
            data.set(pos, map_key(row[x]));
            mask.set(pos, Bool(true));
        }
    }

    // TODO: better paste pos
    return CanvasSnippet(std::move(data), std::move(mask), point_zero);
}

void SelectionControl::handle_tool_operation( ToolTarget &target )
{
    if( !target.has_canvas || !target.selection ) {
        start.reset();
        return;
    }
    CanvasSnippet *snippet = target.snippets.get_snippet( target.mapgen.uuid );

    if( snippet && target.selection->has_selected() ) {
        // May happen after an undo
        snippet = nullptr;
        target.snippets.drop_snippet( target.mapgen.uuid );
    }

    const auto apply_snippet = [&]() {
        if( snippet ) {
            snippet = nullptr;
            CanvasSnippet data = target.snippets.drop_snippet( target.mapgen.uuid );
            target.mapgen.apply_snippet( data );
            target.mapgen.select_from_snippet( data );
            target.made_changes = true;
        }
    };
    const auto is_snippet_at = [&]( point_abs_etile pos ) {
        if( !snippet ) {
            return false;
        }
        point pos_is_snippet = pos.raw() - snippet->get_pos();
        return snippet->get_bounds().contains( pos_is_snippet ) &&
               snippet->get_data_at( pos_is_snippet ).has_value();
    };
    const auto is_selection_at = [&]( point_abs_etile pos ) {
        return target.selection->get( pos.raw() );
    };
    if( should_import_from_clipboard ) {
        should_import_from_clipboard = false;
        std::optional<CanvasSnippet> new_snippet = try_import_snippet_from_clipboard();
        if (new_snippet) {
            // Abort & paste
            apply_snippet();
            start.reset();
            selection_aborted = true;

            if (target.selection->has_selected()) {
                target.selection->clear_all();
                target.made_changes = true;
            }

            // TODO: paste pos
            target.snippets.add_snippet(target.mapgen.uuid, std::move(*new_snippet));
        }
    }
    if( target.view_hovered ) {
        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
            dismissing_selection = true;
            drag_start = target.cursor_tile_pos;
            if( !ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
                if( is_snippet_at( target.cursor_tile_pos ) ) {
                    is_dragging_snippet = true;
                    initial_snippet_pos = snippet->get_pos();
                } else if( is_selection_at( target.cursor_tile_pos ) ) {
                    is_dragging_selection = true;
                }
            }
        }
        if( ImGui::IsMouseDragging( ImGuiMouseButton_Left ) ) {
            if( is_dragging_selection ) {
                // Convert selection to snippet
                CanvasSnippet new_snippet = make_snippet( target.mapgen.base.canvas, *target.selection );
                initial_snippet_pos = new_snippet.get_pos();
                is_dragging_selection = false;
                is_dragging_snippet = true;
                target.snippets.add_snippet( target.mapgen.uuid, std::move( new_snippet ) );
                target.mapgen.erase_selected( *target.selection );
                target.selection->clear_all();
                target.made_changes = true;
            } else if( is_dragging_snippet ) {
                point drag_delta = target.cursor_tile_pos.raw() - drag_start->raw();
                snippet->set_pos( *initial_snippet_pos + drag_delta );
            } else if( !start && !selection_aborted ) {
                // Stroke start
                start = drag_start;
                dismissing_selection = false;
            }
        }
        if( ImGui::IsMouseReleased( ImGuiMouseButton_Left ) ) {
            if( start ) {
                // Stroke end
                apply_snippet();
                point_abs_etile p1 = *start;
                point_abs_etile p2 = get_rectangle_end( target );

                std::vector<point> rect = make_rectangle( p1, p2 );
                if( !ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
                    target.selection->clear_all();
                }
                apply( *target.selection, rect );
                target.made_changes = true;
                start.reset();
            } else if( dismissing_selection ) {
                dismissing_selection = false;
                if( !is_dragging_selection && !is_dragging_snippet ) {
                    // Dismissing does not affects clicks inside the selected area.
                    apply_snippet();
                    if( !ImGui::IsKeyDown( ImGuiKey_ModShift ) ) {
                        if( target.selection->has_selected() ) {
                            target.selection->clear_all();
                            target.made_changes = true;
                        }
                    }
                }
            }
        }
        if( ImGui::IsKeyPressed( ImGuiKey_Delete ) && target.selection->has_selected() ) {
            // Abort & delete
            start.reset();
            selection_aborted = true;

            target.mapgen.erase_selected( *target.selection );
            target.made_changes = true;
        }
        if( snippet ) {
            if( ImGui::IsKeyPressed( ImGuiKey_UpArrow ) ||
                ImGui::IsKeyPressed( ImGuiKey_DownArrow ) ||
                ImGui::IsKeyPressed( ImGuiKey_LeftArrow ) ||
                ImGui::IsKeyPressed( ImGuiKey_RightArrow )
              ) {
                // Abort & move snippet
                start.reset();
                selection_aborted = true;

                point delta;
                if( ImGui::IsKeyPressed( ImGuiKey_UpArrow ) ) {
                    delta = point(0, -1);
                } else if( ImGui::IsKeyPressed( ImGuiKey_DownArrow ) ) {
                    delta = point(0, 1);
                } else if( ImGui::IsKeyPressed( ImGuiKey_LeftArrow ) ) {
                    delta = point(-1, 0);
                } else {
                    delta = point(-1, 0);
                }

                snippet->set_pos( snippet->get_pos() + delta );
            }
            if( ImGui::IsKeyPressed( ImGuiKey_Enter ) ) {
                // Abort & apply snippet
                start.reset();
                selection_aborted = true;

                apply_snippet();
                target.mapgen.get_selection_mask()->clear_all();
                target.made_changes = true;
            }
        }
        if( ImGui::IsKeyDown( ImGuiKey_ModCtrl ) ) {
            if( ImGui::IsKeyPressed( ImGuiKey_A ) ) {
                // Abort & select all
                apply_snippet();
                start.reset();
                selection_aborted = true;

                target.selection->set_all();
                target.made_changes = true;
            }
            if( ImGui::IsKeyPressed( ImGuiKey_X ) && target.selection->has_selected() ) {
                // Abort & cut
                apply_snippet();
                start.reset();
                selection_aborted = true;

                CanvasSnippet new_snippet = make_snippet( target.mapgen.base.canvas, *target.selection );
                target.snippets.clipboard = std::move( new_snippet );

                target.mapgen.erase_selected( *target.selection );
                target.made_changes = true;
            }
            if( ImGui::IsKeyPressed( ImGuiKey_C ) && target.selection->has_selected() ) {
                // Abort & copy
                apply_snippet();
                start.reset();
                selection_aborted = true;

                CanvasSnippet new_snippet = make_snippet( target.mapgen.base.canvas, *target.selection );
                target.snippets.clipboard = std::move( new_snippet );
            }
            if( ImGui::IsKeyPressed( ImGuiKey_V ) && target.snippets.clipboard.has_value() ) {
                // Abort & paste
                apply_snippet();
                start.reset();
                selection_aborted = true;

                if( target.selection->has_selected() ) {
                    target.selection->clear_all();
                    target.made_changes = true;
                }

                // TODO: paste pos
                CanvasSnippet paste_data = *target.snippets.clipboard;
                target.snippets.add_snippet( target.mapgen.uuid, std::move( paste_data ) );
            }
        }
        if( ImGui::IsKeyPressed( ImGuiKey_Escape ) ) {
            // Abort / clear selection
            if( start ) {
                start.reset();
                selection_aborted = true;
            } else if( snippet ) {
                snippet = nullptr;
                target.snippets.drop_snippet( target.mapgen.uuid );
            } else if( target.selection->has_selected() ) {
                target.selection->clear_all();
                target.made_changes = true;
            }
        }
        if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
            selection_aborted = false;
        }
        if( start ) {
            target.want_tooltip = true;
            point_abs_etile p1 = *start;
            point_abs_etile p2 = get_rectangle_end( target );
            std::vector<point> rect = make_rectangle( p1, p2 );
            for( const point &p : rect ) {
                target.highlight.tiles.emplace_back( p );
            }
        }
    }
    if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) ) {
        start.reset();
        drag_start.reset();
        initial_snippet_pos.reset();
        dismissing_selection = false;
        is_dragging_snippet = false;
        is_dragging_selection = false;
    }
}

std::vector<point> SelectionControl::make_rectangle( point_abs_etile p1,
        point_abs_etile p2 ) const
{
    // TODO: deduplicate with Rectangle tool
    auto corners = editor::normalize_rect( p1, p2 );
    std::vector<point> ret;
    for( int y = corners.first.y(); y <= corners.second.y(); y++ ) {
        for( int x = corners.first.x(); x <= corners.second.x(); x++ ) {
            ret.emplace_back( x, y );
        }
    }
    return ret;
}

void SelectionControl::apply( SelectionMask &selection, const std::vector<point> &rect )
{
    for( const auto &p : rect ) {
        if( selection.get_bounds().contains( p ) ) {
            selection.set( p );
        }
    }
}

point_abs_etile SelectionControl::get_rectangle_end( ToolTarget &target ) const
{
    return target.cursor_tile_pos;
}

void SelectionControl::show_tooltip( ToolTarget &target )
{
    // TODO: deduplicate with Rectangle tool
    if( !start ) {
        ImGui::Text( "<ERROR:null rectangle start>" );
        return;
    }
    point delta = ( *start - get_rectangle_end( target ) ).raw().abs();
    ImGui::Text( "X %d", delta.x + 1 );
    ImGui::Text( "Y %d", delta.y + 1 );
}

} // namespace editor::tools
