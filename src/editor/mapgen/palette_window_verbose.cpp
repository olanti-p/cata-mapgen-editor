#include "mapgen/palette_window.h"
#include "palette.h"

#include "common/color.h"
#include "mapgen/palette_making.h"
#include "mapgen/piece_impl.h"
#include "mapgen/palette_view.h"
#include "project/project.h"
#include "state/control_state.h"
#include "state/state.h"
#include "state/ui_state.h"
#include "widget/widgets.h"

#include "translations.h"
#include "localized_comparator.h"

#include <unordered_set>

namespace editor
{
static bool is_expanded( const State &state, const Palette& palette, const Piece&piece )
{
    detail::ExpandedPiece key{ palette.uuid, piece.uuid };
    return state.ui->expanded_pieces_source.count(key) != 0;
}

static void expand_piece( State &state, const Palette& palette, const Piece &piece )
{
    detail::ExpandedPiece key{ palette.uuid, piece.uuid };
    state.ui->expanded_pieces_source.insert(key);
}

static void collapse_piece( State &state, const Palette& palette, const Piece &piece )
{
    detail::ExpandedPiece key{ palette.uuid, piece.uuid };
    state.ui->expanded_pieces_source.erase(key);
}

static bool is_expanded(const State& state, const ViewPalette& palette, const ViewPiece& piece)
{
    detail::ExpandedPiece key{ palette.source_uuid, piece.piece->uuid };
    return state.ui->expanded_pieces_resolved.count(key) != 0;
}

static void expand_piece(State& state, const ViewPalette& palette, const ViewPiece& piece)
{
    detail::ExpandedPiece key{ palette.source_uuid, piece.piece->uuid };
    state.ui->expanded_pieces_resolved.insert(key);
}

static void collapse_piece(State& state, const ViewPalette& palette, const ViewPiece& piece)
{
    detail::ExpandedPiece key{ palette.source_uuid, piece.piece->uuid };
    state.ui->expanded_pieces_resolved.erase(key);
}

void show_mapping_source( State &state, editor::Palette &p, editor::PaletteEntry &entry,
                   bool &show )
{
    std::string wnd_id = string_format( "Mappings##wnd-mappings-%d-%s", p.uuid, entry.key.str() );
    ImGui::SetNextWindowSize( ImVec2( 450.0f, 300.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2( 50.0f, 50.0f ), ImGuiCond_FirstUseEver );
    if( !ImGui::Begin( wnd_id.c_str(), &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( entry.key );

    ImGui::HelpMarkerInline(
        "List of data mappings this pallete is assigning to this symbol, fully editable."
        "\n\nDoes not list data inherited from other palettes, refer to \"Resolved\" tab for those."
    );
    ImGui::Text("Source mappings for \"%s\".  %d pieces.", entry.key.str().c_str(), entry.pieces.size());
    ImGui::Text("Owner palette: %s", p.display_name().c_str());
    if (ImGui::Button("Open Loot Designer")) {
        state.ui->toggle_loot_designer_source_mappping(p.uuid, entry.key);
    }

    std::vector<std::unique_ptr<Piece>> &list = entry.pieces;

    bool changed = ImGui::VectorWidget()
    .with_add( [&]()->bool {
        std::vector<std::pair<std::string, PieceType>> piece_opts;
        for( const auto &it : editor::get_piece_templates() )
        {
            PieceType pt = it->get_type();
            if( !is_available_as_mapping( pt ) ) {
                continue;
            }
            if( is_piece_exclusive( pt ) && entry.has_piece_of_type( pt ) ) {
                continue;
            }
            piece_opts.emplace_back( io::enum_to_string<PieceType>( pt ), pt );
        }

        std::sort( piece_opts.begin(), piece_opts.end(), []( const auto & a, const auto & b ) -> bool {
            return localized_compare( a, b );
        } );

        std::string new_piece_str;
        new_piece_str += "Add mapping...";
        new_piece_str += '\0';
        for( const auto &it : piece_opts )
        {
            new_piece_str += it.first;
            new_piece_str += '\0';
        }
        bool ret = false;
        int new_piece_type = 0;
        if( ImGui::Combo( "##pick-new-mapping", &new_piece_type, new_piece_str.c_str() ) )
        {
            if( new_piece_type != 0 ) {
                auto ptr = editor::make_new_piece( piece_opts[new_piece_type - 1].second );
                UUID uuid = state.project().uuid_generator();
                ptr->uuid = uuid;
                ptr->init_new();
                Piece* ptr_raw = ptr.get();
                list.push_back( std::move( ptr ) );
                expand_piece( state, p, *ptr_raw );
                ret = true;
            }
        }
        return ret;
    } )
    .with_for_each( [&]( size_t idx ) {
        Piece& piece = *list[idx].get();
        if( is_expanded( state, p, piece ) ) {
            if( ImGui::ArrowButton( "##collapse", ImGuiDir_Down ) ) {
                collapse_piece( state, p, piece );
            }
            ImGui::HelpPopup( "Hide details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), piece.fmt_summary().c_str() );
            if (piece.constraint) {
                ImGui::SeparatorText("$ CONDITIONAL PIECE $");
            }
            piece.show_ui( state );
            ImGui::Separator();
        } else {
            if( ImGui::ArrowButton( "##expand", ImGuiDir_Right ) ) {
                expand_piece( state, p, piece );
            }
            ImGui::HelpPopup( "Show details." );
            ImGui::SameLine();
            ImGui::Text( "%d %s", static_cast<int>( idx ), piece.fmt_summary().c_str() );
        }
    } )
    .with_can_duplicate( [&]( size_t idx ) -> bool {
        return !editor::is_piece_exclusive( list[idx]->get_type() );
    } )
    .with_duplicate( [&]( size_t idx ) {
        list.insert( std::next( list.cbegin(), idx + 1 ), list[idx]->clone() );
    } )
    .with_default_delete()
    .with_default_move()
    .with_default_drag_drop()
    .run( list );

    if( changed ) {
        state.mark_changed();
    }

    if( state.is_changed() ) {
        entry.sprite_cache_valid = false;
    }

    ImGui::PopID();
    ImGui::End();
}

void show_mapping_resolved(State& state, editor::ViewPalette& p, editor::ViewEntry& entry,
    bool& show)
{
    std::string wnd_id = string_format("Resolved Mappings##wnd-resolved-mappings-%d-%s", p.source_uuid, entry.key.str());
    ImGui::SetNextWindowSize(ImVec2(450.0f, 300.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(wnd_id.c_str(), &show)) {
        ImGui::End();
        return;
    }
    ImGui::PushID(entry.key);
    ImGui::HelpMarkerInline(
        "List of data mappings this pallete and all its active ancestors are assigning to this symbol."
        "\n\nRefer to \"Source\" tabs of original pallettes to add/delete/reorder mappings."
        "\n\nRefer to \"General\" tab of this palette to configure which ancestors are active."
    );
    ImGui::Text("Resolved mappings for \"%s\".  %d pieces.", entry.key.str().c_str(), entry.pieces.size());
    ImGui::Text("Origin palette: %s", state.project().get_palette(p.source_uuid)->display_name().c_str());
    if (ImGui::Button("Open Loot Designer")) {
        state.ui->toggle_loot_designer_resolved_mappping(p.source_uuid, entry.key);
    }
    
    std::vector<editor::ViewPiece>& list = entry.pieces;

    bool changed = ImGui::VectorWidget()
        .with_for_each([&](size_t idx) {
        ViewPiece& vp = list[idx];
        if (is_expanded(state, p, vp)) {
            if (ImGui::ArrowButton("##collapse", ImGuiDir_Down)) {
                collapse_piece(state, p, vp);
            }
            ImGui::HelpPopup("Hide details.");
            ImGui::SameLine();
            ImGui::Text("%d %s", static_cast<int>(idx), vp.piece->fmt_summary().c_str());
            ImGui::Text("From %s", vp.pal->display_name().c_str());
            if (vp.piece->constraint) {
                ImGui::SeparatorText("$ CONDITIONAL PIECE $");
            }
            vp.piece->show_ui(state);
            ImGui::Separator();
        }
        else {
            if (ImGui::ArrowButton("##expand", ImGuiDir_Right)) {
                expand_piece(state, p, vp);
            }
            ImGui::HelpPopup("Show details.");
            ImGui::SameLine();
            ImGui::Text("%d %s", static_cast<int>(idx), vp.piece->fmt_summary().c_str());
        }
            })
        .run(list);

    if (changed) {
        state.mark_changed();
    }

    if (state.is_changed()) {
        entry.sprite_cache_valid = false;
    }

    ImGui::PopID();
    ImGui::End();
}

static bool show_palette_add_entry_section( State &state, Palette &palette,
        std::vector<PaletteEntry> &list )
{
    bool ret = false;
    ControlState &control = *state.control;
    QuickPaletteAddState &qstate = control.quick_add_state;
    if( qstate.palette == palette.uuid ) {
        qstate.active = true;
        switch( qstate.mode ) {
            case QuickAddMode::Ter: {
                if( ImGui::ImageButton( "cancel", "me_delete" ) ) {
                    qstate.active = false;
                }
                ImGui::HelpPopup( "Cancel" );
                ImGui::SameLine();
                if( ImGui::InputId( "Terrain", qstate.eid_ter ) ) {
                    list.emplace_back( make_simple_entry( state.project(), palette, &qstate.eid_ter, nullptr ) );
                    ret = true;
                    qstate.active = false;
                }
                break;
            }
            case QuickAddMode::Furn: {
                if( ImGui::ImageButton( "cancel", "me_delete" ) ) {
                    qstate.active = false;
                }
                ImGui::HelpPopup( "Cancel" );
                ImGui::SameLine();
                if( ImGui::InputId( "Furniture", qstate.eid_furn ) ) {
                    list.emplace_back( make_simple_entry( state.project(), palette, nullptr, &qstate.eid_furn ) );
                    ret = true;
                    qstate.active = false;
                }
                break;
            }
            case QuickAddMode::Both: {
                if( ImGui::ImageButton( "cancel", "me_delete" ) ) {
                    qstate.active = false;
                }
                ImGui::HelpPopup( "Cancel" );
                ImGui::SameLine();
                ImGui::InputId( "Furniture", qstate.eid_furn );
                bool disabled = !qstate.eid_furn.is_valid() || !qstate.eid_ter.is_valid();
                ImGui::BeginDisabled( disabled );
                if( ImGui::ImageButton( "confirm", "me_add" ) ) {
                    list.emplace_back( make_simple_entry( state.project(), palette, &qstate.eid_ter, &qstate.eid_furn ) );
                    ret = true;
                    qstate.active = false;
                }
                ImGui::EndDisabled();
                ImGui::HelpPopup( "Confirm" );
                ImGui::SameLine();
                ImGui::InputId( "Terrain", qstate.eid_ter );
                break;
            }
            default: {
                ImGui::Text( "<Not Implemented>" );
                break;
            }
        }
    } else {
        // In default mode
        if( ImGui::Button( "New Empty" ) ) {
            list.emplace_back( make_simple_entry( state.project(), palette, nullptr, nullptr ) );
            ret = true;
        }
        ImGui::HelpPopup( "Add a new empty entry." );
        ImGui::SameLine();
        if( ImGui::Button( "New Ter" ) ) {
            qstate = QuickPaletteAddState();
            qstate.active = true;
            qstate.palette = palette.uuid;
            qstate.mode = QuickAddMode::Ter;
        }
        ImGui::HelpPopup( "Add a new entry with NO furniture and 1 terrain option." );
        ImGui::SameLine();
        if( ImGui::Button( "New Furn" ) ) {
            qstate = QuickPaletteAddState();
            qstate.active = true;
            qstate.palette = palette.uuid;
            qstate.mode = QuickAddMode::Furn;
        }
        ImGui::HelpPopup( "Add a new entry with 1 furniture option and NO terrain underneath." );
        ImGui::SameLine();
        if( ImGui::Button( "New Furn+Ter" ) ) {
            qstate = QuickPaletteAddState();
            qstate.active = true;
            qstate.palette = palette.uuid;
            qstate.mode = QuickAddMode::Both;
        }
        ImGui::HelpPopup( "Add a new entry with 1 furniture option and 1 terrain option." );
        // 2-in-1 hack: provides helpful hint AND keeps window alignment for Furn+Ter quick action
        ImGui::Text( "You can fully customize entry data after creation." );
    }
    return ret;
}

static void show_palette_entries_verbose( State &state, Palette &palette )
{
    std::vector<PaletteEntry> &list = palette.entries;

    ImGui::Text("%d symbols  %d mappings", list.size(), palette.num_pieces_total());
    ImGui::Separator();

    Project &proj = state.project();
    ToolsState &tools = *state.ui->tools;

    bool changed = ImGui::VectorWidget()
    .with_add( [&]() -> bool {
        return show_palette_add_entry_section( state, palette, list );
    } )
    .with_duplicate( [&]( size_t idx ) {
        const PaletteEntry &src = list[ idx ];
        PaletteEntry new_entry = src;
        new_entry.key = pick_available_key(palette);
        new_entry.sprite_cache_valid = false;
        list.insert( std::next( list.cbegin(), idx + 1 ), std::move(new_entry) );
    } )
    .with_delete( [&]( size_t idx ) {
        const MapKey &uuid = list[ idx ].key;
        if( tools.get_main_tile() == uuid ) {
            tools.set_main_tile( MapKey() );
        }
        list.erase( std::next( list.cbegin(), idx ) );
    } )
    .with_for_each( [&]( size_t idx ) {
        bool selected = list[idx].key == tools.get_main_tile();

        if (false) {
            // FIXME: implement colors
            if (ImGui::ColorEdit4("MyColor##3", (float*)&list[idx].color,
                ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                state.mark_changed("palette-entry-color");
            }
            ImGui::SameLine();
        }

        // Use dim colors so it's not distracting
        ImVec4 c = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        c.w *= 0.6f;
        ImGui::PushStyleColor(ImGuiCol_Button, c);
        c = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        c.w *= 0.6f;
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c);
        c = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
        c.w *= 0.6f;
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, c);

        std::string buf = list[idx].key.str() + "###edit-key-button";
        // TODO: move these statics to control state or something
        static std::string symbol_edit_buffer;
        static bool grab_input_focus = false;
        static std::string symbol_error_msg;
        if (ImGui::Button(buf.c_str(), ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()))) {
            ImGui::OpenPopup("edit-key-popup");
            symbol_edit_buffer = list[idx].key.str();
            grab_input_focus = true;
            symbol_error_msg.clear();
        }
        if (ImGui::BeginPopup("edit-key-popup")) {
            if (grab_input_focus) {
                grab_input_focus = false;
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputSymbol("New symbol", symbol_edit_buffer);
            bool apply = false;
            bool apply_everywhere = false;
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                apply = true;
            }
            if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                apply_everywhere = true;
            }
            if (ImGui::Button("Cancel [Esc]") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply [Enter]")) {
                apply = true;
            }
            ImGui::HelpPopup("Change to new symbol in this palette.");
            /** FIXME: implement
            if (ImGui::Button("Apply everywhere [Ctrl+Enter]")) {
                apply_everywhere = true;
            }
            ImGui::HelpPopup("Change to new symbol in this palette,\nin all mapgens that use this palette, and in all palettes that\ninherit from this palette or are inherited by this palette.");
            */
            if (!symbol_error_msg.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, col_error_text);
                ImGui::Text("%s", symbol_error_msg.c_str());
                ImGui::PopStyleColor();
            }

            if (apply || apply_everywhere) {
                std::u32string u32s = utf8_to_utf32(symbol_edit_buffer);
                MapKey new_key = MapKey(symbol_edit_buffer);
                bool symbol_exists = false;
                for (const auto& entry : list) {
                    if (&entry == &list[idx]) {
                        continue;
                    }
                    if (entry.key == new_key) {
                        symbol_exists = true;
                        break;
                    }
                }
                if (symbol_edit_buffer.empty()) {
                    symbol_error_msg = "Symbol cannot be empty";
                    grab_input_focus = true;
                } else if (symbol_exists) {
                    symbol_error_msg = "Symbol " + new_key.str() + " is already in use";
                    grab_input_focus = true;
                } else if (u32s.size() != 1) {
                    // FIXME: this won't show up because the widget already truncates
                    symbol_error_msg = "Must be 1 unicode character";
                    grab_input_focus = true;
                } else if (new_key == list[idx].key) {
                    // No changes
                    ImGui::CloseCurrentPopup();
                } else {
                    list[idx].key = new_key;
                    state.mark_changed();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::HelpPopup( "Symbol to use on canvas.\nClick to change." );
        if (selected) {
            // Highlight entry
            ImVec4 c = col_selected_palette_entry;
            c.w *= 0.6f;
            ImGui::PushStyleColor(ImGuiCol_Button, c);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, c);
        }
        bool clicked = false;
        PieceAltTerrain* piece_disp_ter = nullptr;
        PieceAltFurniture* piece_disp_furn = nullptr;
        {
            std::optional<std::string> text;
            piece_disp_ter = list[idx].get_first_piece_of_type<PieceAltTerrain>();
            if(piece_disp_ter) {
                text = piece_disp_ter->fmt_data_summary();
            }
            ImGui::SameLine();
            ImGui::PushID("ter");
            clicked = ImGui::Button(
                text ? text->c_str() : "-",
                ImVec2( ImGui::GetFrameHeight() * 8.0f, 0.0f )
            );
            ImGui::PopID();
        }
        {
            std::optional<std::string> text;
            piece_disp_furn = list[idx].get_first_piece_of_type<PieceAltFurniture>();
            if(piece_disp_furn) {
                text = piece_disp_furn->fmt_data_summary();
            }
            // Reduce gap between ter and furn widgets
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
            ImGui::SameLine();
            ImGui::PushID("furn");
            clicked = ImGui::Button(
                text ? text->c_str() : "-",
                ImVec2( ImGui::GetFrameHeight() * 8.0f, 0.0f )
            ) || clicked;
            ImGui::PopID();
            ImGui::PopStyleVar();
        }
        if (selected) {
            ImGui::PopStyleColor(3);
        }
        ImGui::PopStyleColor(3);
        if (clicked) {
            if (selected) {
                tools.set_main_tile(MapKey());
            }
            else {
                tools.set_main_tile(list[idx].key);
            }
        }
        ImGui::SameLine();
        if( ImGui::ArrowButton( "##mapping", ImGuiDir_Right ) ) {
            state.ui->toggle_show_source_mapping( palette.uuid, list[idx].key );
        }
        ImGui::HelpPopup( "Show/hide mappings\nassociated with this symbol." );

        int additional_pieces = 0;
        std::string additional_summary;
        for( const auto &it : list[idx].pieces ) {
            Piece* og_piece = it.get();
            if (og_piece == piece_disp_ter || og_piece == piece_disp_furn) {
                continue;
            }
            additional_pieces += 1;
            additional_summary += it->fmt_summary();
            additional_summary += "\n";
        }
        if( additional_pieces > 0 ) {
            ImGui::SameLine();
            ImGui::Text( "+ %d", additional_pieces );
            ImGui::HelpPopup( additional_summary.c_str() );
        }
    } )
    .with_default_move()
    .with_drag_drop( [&]( size_t idx ) -> bool {
        return handle_palette_entry_drag_and_drop( state.project(), palette, idx );
    } )
    .run( list );

    if( changed ) {
        state.mark_changed();
    }
}

// FIXME: this is an almost complete dupe of the other function
static void show_palette_entries_verbose(State& state, ViewPalette& palette)
{
    std::vector<ViewEntry>& list = palette.entries;

    ImGui::Text("%d symbols  %d mappings", list.size(), palette.num_pieces_total());
    ImGui::Separator();

    Project& proj = state.project();
    ToolsState& tools = *state.ui->tools;

    bool changed = ImGui::VectorWidget()
        .with_for_each([&](size_t idx) {
        bool selected = list[idx].key == tools.get_main_tile();

        ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
        ImGui::BeginDisabled();
        std::string buf = list[idx].key.str();
        if (ImGui::InputSymbol("##key", buf, default_editor_map_key.str().c_str())) {
            // FIXME: ensure unique keys, sync to ancestors
            state.mark_changed("palette-entry-key");
        }
        ImGui::EndDisabled();
        ImGui::HelpPopup("Symbol to use on canvas.");
        ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 2);
        if (selected) {
            // Highlight entry
            ImVec4 c = col_selected_palette_entry;
            c.w *= 0.6f;
            ImGui::PushStyleColor(ImGuiCol_Button, c);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, c);
        }
        else {
            // Use dim colors so it's not distracting
            ImVec4 c = ImGui::GetStyleColorVec4(ImGuiCol_Button);
            c.w *= 0.6f;
            ImGui::PushStyleColor(ImGuiCol_Button, c);
            c = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
            c.w *= 0.6f;
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c);
            c = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
            c.w *= 0.6f;
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, c);
        }
        bool clicked = false;
        const PieceAltTerrain* piece_disp_ter = nullptr;
        const PieceAltFurniture* piece_disp_furn = nullptr;
        {
            std::optional<std::string> text;
            piece_disp_ter = list[idx].get_first_piece_of_type<PieceAltTerrain>();
            if (piece_disp_ter) {
                text = piece_disp_ter->fmt_data_summary();
            }
            ImGui::SameLine();
            ImGui::PushID("ter");
            clicked = ImGui::Button(
                text ? text->c_str() : "-",
                ImVec2(ImGui::GetFrameHeight() * 8.0f, 0.0f)
            );
            ImGui::PopID();
        }
        {
            std::optional<std::string> text;
            piece_disp_furn = list[idx].get_first_piece_of_type<PieceAltFurniture>();
            if (piece_disp_furn) {
                text = piece_disp_furn->fmt_data_summary();
            }
            ImGui::SameLine();
            ImGui::PushID("furn");
            clicked = ImGui::Button(
                text ? text->c_str() : "-",
                ImVec2(ImGui::GetFrameHeight() * 8.0f, 0.0f)
            ) || clicked;
            ImGui::PopID();
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        if (clicked) {
            if (selected) {
                tools.set_main_tile(MapKey());
            }
            else {
                tools.set_main_tile(list[idx].key);
            }
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##mapping", ImGuiDir_Right)) {
            const ViewEntry& ve = palette.entries[idx];
            state.ui->toggle_show_resolved_mapping(palette.source_uuid, list[idx].key);
        }
        ImGui::HelpPopup("Show/hide mappings\nassociated with this symbol.");

        int additional_pieces = 0;
        std::string additional_summary;
        for (const auto& it : list[idx].pieces) {
            const Piece* og_piece = it.piece;
            if (og_piece == piece_disp_ter || og_piece == piece_disp_furn) {
                continue;
            }
            additional_pieces += 1;
            additional_summary += og_piece->fmt_summary();
            additional_summary += "\n";
        }
        if (additional_pieces > 0) {
            ImGui::SameLine();
            ImGui::Text("+ %d", additional_pieces);
            ImGui::HelpPopup(additional_summary.c_str());
        }
            })
        .run(list);

    if (changed) {
        state.mark_changed();
    }
}

struct PaletteTreeState {
    ViewPaletteTreeState& view;
    std::optional<std::string> import_pal;

    size_t get_selected_idx(UUID palette, size_t ancestor_idx)
    {
        auto& list = view.selected_opts[palette];
        if (list.size() <= ancestor_idx) {
            list.resize(ancestor_idx+1);
        }
        return list[ancestor_idx];
    }
    void set_selected_idx(UUID palette, size_t ancestor_idx, size_t selected_idx)
    {
        view.selected_opts[palette][ancestor_idx] = selected_idx;
    }
};

void print_dependency_list(PaletteTreeState& pts, State& state, Palette& p, bool*selected);

static void print_palette_entry_resolved(PaletteTreeState& pts, State& state, Palette& p2, bool* selected)
{
    std::string disp_name = p2.display_name();
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (p2.ancestors.list.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (*selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNodeEx(disp_name.c_str(), flags)) {
        print_dependency_list(pts, state, p2, selected);
        ImGui::TreePop();
    }
    if (ImGui::IsItemClicked()) {
        *selected = true;
    }
}

static void print_palette_entry_unresolved(PaletteTreeState& pts, State& state, const std::string& maybe_id, bool* selected)
{
    if (ImGui::SmallButton("Import")) {
        pts.import_pal = maybe_id;
    }
    ImGui::SameLine();
    ImGui::Selectable(maybe_id.c_str(), selected);
}

static void print_palette_entry(PaletteTreeState& pts, State& state, const std::string& maybe_id, bool *selected) {
    ImGui::PushID(maybe_id.c_str());
    Palette* p = state.project().find_palette_by_string(maybe_id);
    if (p) {
        print_palette_entry_resolved(pts, state, *p, selected);
    }
    else {
        print_palette_entry_unresolved(pts, state, maybe_id, selected);
    }
    ImGui::PopID();
}

void print_dependency_list(PaletteTreeState& pts, State& state, Palette& p, bool* selected)
{
    ImGui::PushID(&p);
    for (size_t ancestor_idx = 0; ancestor_idx < p.ancestors.list.size(); ancestor_idx++) {
        const PaletteAncestorSwitch& sw = p.ancestors.list[ancestor_idx];
        ImGui::PushID(&sw);
        if (sw.options.size() == 1) {
            print_palette_entry(pts, state, sw.options[0], selected);
        }
        else {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("SWITCH", flags)) {
                for (size_t opt_idx = 0; opt_idx < sw.options.size(); opt_idx++) {
                    const std::string& s = sw.options[opt_idx];
                    ImGui::PushID(&s);
                    bool opt_selected = (pts.get_selected_idx(p.uuid, ancestor_idx) == opt_idx) && *selected;
                    print_palette_entry(pts, state, s, &opt_selected);
                    if (opt_selected) {
                        pts.set_selected_idx(p.uuid, ancestor_idx, opt_idx);
                        *selected = true;
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

static void show_palette_header_info( State& state, Palette& p)
{
    if (ImGui::InputText("Display name", &p.name)) {
        state.mark_changed("palette-name");
    }
    ImGui::HelpPopup("Only used inside the editor.");
    if (p.imported) {
        ImGui::Text("ID: %s", p.imported_id.data.c_str());
        ImGui::HelpPopup("ID from which the palette was imported.");
        ImGui::SameLine();
        if (ImGui::Button("Reimport")) {
            reimport_palette(state, p);
            state.mark_changed("palette-reimported");
        }
        ImGui::HelpPopup("Discard all changes and re-import the palette from same id.");
        ImGui::SameLine();
        if (ImGui::Button("+ Recursive import")) {
            reimport_palette(state, p);
            recursive_import_palette(state, p);
            state.mark_changed("palette-recursive-imported");
        }
        ImGui::HelpPopup("Same as Reimport, but also ensures all ancestors are imported.");
        if (p.temp_palette) {
            ImGui::Text("EMBEDDED PALETTE, CAN ONLY BE USED BY OWNER MAPGEN");
        }
        else {
            ImGui::Text("IMPORTED PALETTE, ALL CHANGES WILL BE DISCARDED ON EXPORT");
        }
    }
    else {
        if (ImGui::InputText("ID", &p.created_id)) {
            state.mark_changed("palette-created-id");
        }
        ImGui::HelpPopup("ID with which to save this palette.");
        if (ImGui::Checkbox("Standalone", &p.standalone)) {
            state.mark_changed("palette-standalone");
        }
        ImGui::HelpPopup("Standalone palettes will be saved with unique id.\nEmbedded palettes will be copied into each mapgen that uses them, and can't be depended on.");
        if (p.standalone) {
            EID::Palette pal_id(p.created_id);
            bool collision = pal_id.is_valid();
            if (!collision) {
                for (const Palette& pal : state.project().palettes) {
                    if (&pal != &p && pal.standalone && pal.created_id == p.created_id) {
                        collision = true;
                        break;
                    }
                }
            }
            if (collision) {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, col_error_text);
                ImGui::Text("ERROR: ID already exists");
                ImGui::PopStyleColor();
            }
        }
    }
    if (!p.ancestors.list.empty()) {
        PaletteTreeState pts{ state.ui->view_palette_tree_states[p.uuid]  };
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
        if (ImGui::TreeNodeEx("Dependency graph", flags)) {
            bool tmp = true;
            print_palette_entry_resolved(pts, state, p, &tmp);
            ImGui::TreePop();
        }
        if (pts.import_pal && EID::Palette(*pts.import_pal).is_valid()) {
            quick_import_palette(state, EID::Palette(*pts.import_pal));
            state.mark_changed();
        }
    }
    else {
        ImGui::Text("<No inheritance>");
    }
}

void show_active_palette_details( State &state, Palette &p, bool &show )
{
    ImGui::SetNextWindowSize( ImVec2( 620.0f, 500.0f ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos( ImVec2( 50.0f, 50.0f ), ImGuiCond_FirstUseEver );

    if( !ImGui::Begin("Active palette details", &show ) ) {
        ImGui::End();
        return;
    }
    ImGui::PushID( p.uuid );

    if (ImGui::BeginTabBar("PaletteDetailsTabBar")) {
        // FIXME: use existing resolved palette, if available
        // FIXME: this is opaque, but we create the palette here because it validates global ui state
        ViewPalette vp(state.project());
        vp.add_palette_recursive(p, state.ui->view_palette_tree_states[p.uuid]);
        vp.finalize();

        if (ImGui::BeginTabItem("General")) {
            show_palette_header_info(state, p);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Source")) {
            show_palette_entries_verbose(state, p);
            show_palette_entries_simple(state, p);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Resolved")) {
            show_palette_entries_verbose(state, vp);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    
    ImGui::PopID();
    ImGui::End();
}

void show_palette_preview( State& state, Palette& p, bool& show )
{
    ImGui::SetNextWindowSize(ImVec2(620.0f, 500.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

    std::string name = p.display_name();
    std::string wnd_id = string_format("Preview for %s###palette-%d-preview", name, p.uuid);
    if (!ImGui::Begin(wnd_id.c_str(), &show)) {
        ImGui::End();
        return;
    }
    ImGui::PushID(p.uuid);

    if (ImGui::BeginTabBar("PalettePreviewTabBar")) {
        // FIXME: use existing resolved palette, if available
        // FIXME: this is opaque, but we create the palette here because it validates global ui state
        ViewPalette vp(state.project());
        vp.add_palette_recursive(p, state.ui->view_palette_tree_states[p.uuid]);
        vp.finalize();

        if (ImGui::BeginTabItem("General")) {
            show_palette_header_info(state, p);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Source")) {
            show_palette_entries_verbose(state, p);
            show_palette_entries_simple(state, p);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Resolved")) {
            show_palette_entries_verbose(state, vp);
            show_palette_entries_simple(state, vp);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::PopID();
    ImGui::End();
}

} // namespace editor
