#include "mapgen.h"

#include <cassert>
#include <cfloat>
#include <climits>
#include <vector>

#include "common/canvas_2d.h"
#include "common/uuid.h"
#include <imgui/imgui.h>
#include "mapgen/canvas_snippet.h"
#include "mapgen/palette.h"
#include "point.h"
#include "selection_mask.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "string_formatter.h"
#include "widget/editable_id.h"
#include "widget/widgets.h"
#include "project/project.h"

namespace editor
{

static void show_canvas_hint()
{
    ImGui::Text( "Use mouse to paint canvas with palette entries." );
}

static void show_flags(State& state, Mapgen& mapgen)
{
    ImGui::SeparatorText("Flags");
    if (ImGui::Checkbox("AVOID_CREATURES", &mapgen.flags.avoid_creatures)) {
        state.mark_changed();
    }
    ImGui::HelpPopup("If a creature is present, terrain, furniture and traps won't be placed.");

    if (ImGui::Checkbox("NO_UNDERLYING_ROTATE", &mapgen.flags.no_underlying_rotate)) {
        state.mark_changed();
    }
    ImGui::HelpPopup("The map won't be rotated even if the underlying overmap tile is.");

    ImGui::HelpMarkerInline(
        "When the mapgen generate terrain on top of existing tile, the tile may already contain preexisting furniture, traps and items. "
        "The following options provide a mechanism for specifying the behaviour to follow in such situations. "
        "It is an error if existing furniture, traps or items are encountered but no behaviour has been given.\n\n"
        "Allow: retains preexisting furniture, traps and items without triggering an error.\n\n"
        "Dismantle: causes any furniture to be deconstructed or bashed, while traps are disarmed.\n"
        "The outputs, along with any other preexisting items, are then retained.\n\n"
        "Erase: removes all preexisting furniture, traps and items before changing the terrain."
    );
    ImGui::Text("Terrain layering policy");
    if (ImGui::ComboEnum("Furniture", mapgen.flags.rule_furn)) {
        state.mark_changed();
    }
    if (ImGui::ComboEnum("Traps", mapgen.flags.rule_traps)) {
        state.mark_changed();
    }
    if (ImGui::ComboEnum("Items", mapgen.flags.rule_items)) {
        state.mark_changed();
    }
}

void show_mapgen_info( State &state, Mapgen &mapgen, bool &show )
{
    if( !ImGui::Begin( "Mapgen Info", &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( mapgen.uuid );

    if( ImGui::InputText( "Display Name", &mapgen.name ) ) {
        state.mark_changed( "mapgen-name" );
    }
    ImGui::HelpPopup( "Only used inside the editor." );

    if (ImGui::PaletteSelector("Palette", mapgen.base.palette, state.project().palettes)) {
        state.mark_changed("mapgen-palette");
    }
    ImGui::Separator();

    if( mapgen.mtype == MapgenType::Oter ) {
        if( ImGui::Checkbox( "Matrix Mode", &mapgen.oter.matrix_mode ) ) {
            state.mark_changed();
        }
        ImGui::HelpPopup(
            "In Normal Mode, the mapgen must be of size 24x24 and can be assigned to 1 or multiple OMTs.  "
            "Recommended for small static specials / buildings, wilderness and mutable specials.\n\n"
            "In Matrix Mode, the mapgen size will be multiple of 24x24, and each 24x24 chunk within it corresponds to a single OMT.  "
            "Recommended for large static specials / buildings."
        );
        if( mapgen.oter.matrix_mode ) {
            // Matrix
            Canvas2D<EID::OterType> &oters = mapgen.oter.om_terrain_matrix;

            point new_size = oters.get_size();
            ImGui::InputInt( "x", &new_size.x );
            ImGui::InputInt( "y", &new_size.y );
            new_size.x = clamp( new_size.x, 1, 16 );
            new_size.y = clamp( new_size.y, 1, 16 );
            if( new_size != oters.get_size() ) {
                state.mark_changed( "oter-matrix-size" );
                oters.set_size( new_size );
                mapgen.set_canvas_size( new_size * 24 );
            }

            ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;
            if( ImGui::BeginTable( "omt-matrix", oters.get_size().x, flags ) ) {
                for( int y = 0; y < oters.get_size().y; y++ ) {
                    ImGui::TableNextRow();
                    ImGui::PushID( y );
                    for( int x = 0; x < oters.get_size().x; x++ ) {
                        ImGui::TableSetColumnIndex( x );
                        ImGui::PushID( x );
                        ImGui::SetNextItemWidth( -FLT_MIN );
                        if( ImGui::InputId( "###id-input", oters.get( point( x, y ) ) ) ) {
                            state.mark_changed();
                        }
                        ImGui::PopID();
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        } else {
            // Single mapgen
            std::vector<EID::OterType> &oters = mapgen.oter.om_terrain;
            std::string label = oters.empty() ? "<none>" : oters[0].data;
            if( oters.size() > 1 ) {
                label += string_format( " +%d", oters.size() - 1 );
            }
            label += "###oter-list-header";
            bool header_expanded = ImGui::CollapsingHeader( label.c_str() );
            ImGui::HelpPopup( "Overmap terrain types this mapgen is assigned to." );
            if( header_expanded ) {
                bool oters_changed = ImGui::VectorWidget()
                .with_for_each( [&]( size_t idx ) {
                    ImGui::PushID( idx );
                    if( ImGui::InputId( "tmp", oters[idx] ) ) {
                        state.mark_changed();
                    }
                    ImGui::PopID();
                } )
                .with_default_add()
                .with_default_delete()
                .with_default_move()
                .with_default_drag_drop()
                .run( oters );

                if( oters_changed ) {
                    state.mark_changed();
                }
            }
        }
        if( ImGui::InputIntClamped( "weight", mapgen.oter.weight, 0, 10000 ) ) {
            state.mark_changed( "mapgen-info-oter-weight-input" );
        }
        ImGui::HelpPopup(
            "Weight of this mapgen, defaults to 100.\n\n"
            "The higher this value is, the more frequently this mapgen will be chosen "
            "to generate the overmap terrain."
        );
        if( ImGui::InputIntRange( "rotation", mapgen.oter.rotation ) ) {
            state.mark_changed();
        }
        ImGui::Text( "Oter mapgen base:" );
        ImGui::HelpPopup( "Defines how to fill in the empty tiles in the canvas.\nEmpty tiles are denoted by symbols ' ' or '.'" );

        if (ImGui::RadioButton("None", mapgen.oter.mapgen_base == OterMapgenFill::None)) {
            mapgen.oter.mapgen_base = OterMapgenFill::None;
            state.mark_changed();
        }
        ImGui::HelpPopup(
            "No fallback.\n\n"
            "All symbols must be defined in the palette."
        );
        ImGui::SameLine();
        ImGui::SameLine();
        if (ImGui::RadioButton("Fallback predecessor mapgen", mapgen.oter.mapgen_base == OterMapgenFill::FallbackPredecessorMapgen)) {
            mapgen.oter.mapgen_base = OterMapgenFill::FallbackPredecessorMapgen;
            state.mark_changed();
        }
        ImGui::HelpPopup(
            "This mapgen will be run on top of a map generated for some other overmap terrain.\n\n"
            "Can be used to generate maps with objects that don't occupy the whole 24x24 area, "
            "and should blend in with the surrounding terrain."
            "For example, an anthill entrance in the woods will generate on top of 'forest' mapgen, "
            "and anthill entrance in the field will generate on top of 'field' mapgen.\n\n"
            "Predecessors are specified in om definitions, this value will be used as fallback.\n\n"
            "Keep in mind that predecessor mapgen may place items, monsters and vehices!"
        );

        if( ImGui::RadioButton( "Fill terrain", mapgen.oter.mapgen_base == OterMapgenFill::FillTer ) ) {
            mapgen.oter.mapgen_base = OterMapgenFill::FillTer;
            state.mark_changed();
        }
        ImGui::HelpPopup(
            "Fill empty tiles with terrain type.\n\n"
            "Useful for maps where most of the terrain is monotonic (e.g. solid rock, or open air, or groundcover)."
        );
        ImGui::SameLine();
        if( ImGui::RadioButton( "Predecessor mapgen",
                                mapgen.oter.mapgen_base == OterMapgenFill::PredecessorMapgen ) ) {
            mapgen.oter.mapgen_base = OterMapgenFill::PredecessorMapgen;
            state.mark_changed();
        }
        ImGui::HelpPopup(
            "This mapgen will be run on top of a map generated for some the specified overmap terrain type.\n\n"
            "Can be used to generate maps with objects that don't occupy the whole 24x24 area, "
            "or maps that are extremely similar to some other maps."
            "For example, a small 8x8 glade in the woods may use 'forest' predecessor mapgen "
            "to generate the greenery, and then place some grass in the center.\n\n"
            "Keep in mind that predecessor mapgen may place items, monsters and vehices!"
        );

        if( mapgen.oter.mapgen_base == OterMapgenFill::PredecessorMapgen ) {
            if( ImGui::InputId( "predecessor_mapgen", mapgen.oter.predecessor_mapgen ) ) {
                state.mark_changed();
            }
            ImGui::HelpPopup( "Overmap type id of predecessor mapgen." );
        } else if (mapgen.oter.mapgen_base == OterMapgenFill::FallbackPredecessorMapgen) {
            if (ImGui::InputId("fallback_predecessor_mapgen", mapgen.oter.predecessor_mapgen)) {
                state.mark_changed();
            }
            ImGui::HelpPopup("Overmap type id of fallback predecessor mapgen.\nTODO: verbose description.");
        }
        else {
            if( ImGui::InputId( "fill_ter", mapgen.oter.fill_ter ) ) {
                state.mark_changed();
            }
            ImGui::HelpPopup( "Terrain type to fill empty spots with." );
        }
        ImGui::HelpMarkerInline("Use mouse to paint canvas with palette entries.");
        if (ImGui::Checkbox("Use Rows", &mapgen.oter.uses_rows)) {
            state.mark_changed();
        }
    } else if( mapgen.mtype == MapgenType::Update ) {
        if( ImGui::InputText( "update_mapgen_id", &mapgen.update.update_mapgen_id ) ) {
            state.mark_changed( "update-mapgen-id" );
        }
        ImGui::HelpPopup( "ID of this update mapgen." );
        if( ImGui::InputId( "fill_ter", mapgen.update.fill_ter ) ) {
            state.mark_changed();
        }
        ImGui::HelpPopup( "Terrain type to fill empty spots with." );
        ImGui::HelpMarkerInline("Use mouse to paint canvas with palette entries.");
        if (ImGui::Checkbox("Use Rows", &mapgen.update.uses_rows)) {
            state.mark_changed();
        }
    } else { // MapgenType::Nested
        if( ImGui::InputText( "nested_mapgen_id", &mapgen.nested.nested_mapgen_id ) ) {
            state.mark_changed( "nested-mapgen-id" );
        }
        ImGui::HelpPopup( "ID of this nested mapgen." );
        if( ImGui::InputIntRange( "rotation", mapgen.nested.rotation ) ) {
            state.mark_changed();
        }
        ImGui::HelpPopup( "Allowed rotations." );
        // Only square nested mapgens are possible
        if( ImGui::InputIntClamped( "mapgensize", mapgen.nested.size.x, 1, SEEX * 2 ) ) {
            mapgen.nested.size.y = mapgen.nested.size.x;
            mapgen.set_canvas_size( mapgen.mapgensize().raw() );
            state.mark_changed();
        }
        ImGui::HelpPopup( "Size of this nested mapgen." );
        show_canvas_hint();
    }

    show_flags(state, mapgen);

    ImGui::PopID();
    ImGui::End();
}

std::string Mapgen::display_name() const
{
    if( !name.empty() ) {
        return name;
    } else {
        return string_format( "[uuid=%d]", uuid );
    }
}

point_rel_etile Mapgen::mapgensize() const
{
    if( mtype == MapgenType::Nested ) {
        return point_rel_etile( nested.size );
    } else if( mtype == editor::MapgenType::Oter && oter.uses_rows ) {
        return point_rel_etile( base.canvas.get_size() );
    } else {
        return point_rel_etile( SEEX * 2, SEEY * 2 );
    }
}

void Mapgen::set_canvas_size( point new_size )
{
    base.canvas.set_size( new_size, MapKey() );
    selection_mask = SelectionMask( new_size );
}

SelectionMask *Mapgen::get_selection_mask()
{
    if( uses_rows() ) {
        if( selection_mask.get_size() != mapgensize().raw() ) {
            // Repair broken projects
            // TODO: remove this?
            selection_mask = SelectionMask( mapgensize().raw() );
        }
        return &selection_mask;
    } else {
        return nullptr;
    }
}

void Mapgen::erase_selected( const SelectionMask &mask )
{
    assert( mapgensize().raw() == mask.get_size() );
    assert( uses_rows() );
    for( int y = 0; y < mask.get_size().y; y++ ) {
        for( int x = 0; x < mask.get_size().x; x++ ) {
            point pos( x, y );
            if( mask.get( pos ) ) {
                base.canvas.set( pos, MapKey() );
            }
        }
    }
}

MapgenBase::~MapgenBase() = default;

void MapgenBase::remove_usages( const MapKey &uuid )
{
    for( MapKey &cell : canvas.get_data() ) {
        if( cell == uuid ) {
            cell = MapKey();
        }
    }
}

void Mapgen::apply_snippet( const CanvasSnippet &snippet )
{
    assert( uses_rows() );

    Canvas2D<MapKey> &canvas = base.canvas;

    for( int y = 0; y < snippet.get_size().y; y++ ) {
        for( int x = 0; x < snippet.get_size().x; x++ ) {
            point p_src( x, y );
            std::optional<MapKey> data = snippet.get_data_at( p_src );
            if( data ) {
                point p_dest = snippet.get_pos() + p_src;
                if( canvas.get_bounds().contains( p_dest ) ) {
                    canvas.set( p_dest, *data );
                }
            }
        }
    }
}

void Mapgen::select_from_snippet( const CanvasSnippet &snippet )
{
    assert( uses_rows() );

    selection_mask.clear_all();

    const SelectionMask &snippet_mask = snippet.get_selection_mask();

    for( int y = 0; y < snippet.get_size().y; y++ ) {
        for( int x = 0; x < snippet.get_size().x; x++ ) {
            point p_src( x, y );
            if( snippet_mask.get( p_src ) ) {
                point p_dest = snippet.get_pos() + p_src;
                if( selection_mask.get_bounds().contains( p_dest ) ) {
                    selection_mask.set( p_dest );
                }
            }
        }
    }
}

const MapObject* Mapgen::get_object(UUID uuid) const
{
    for (const auto& obj : objects) {
        if (obj.piece->uuid == uuid) {
            return &obj;
        }
    }
    return nullptr;
}

} // namespace editor
