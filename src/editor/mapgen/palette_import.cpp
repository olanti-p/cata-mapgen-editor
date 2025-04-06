#include "all_enum_values.h"
#include "mapgen/piece_type.h"
#include "palette_making.h"

#include "mapgen/palette.h"
#include "mapgen_map_key.h"
#include "type_id.h"
#include "widget/editable_id.h"
#include "project/project.h"
#include "state/state.h"
#include "state/control_state.h"
#include "state/ui_state.h"

// FIXME: conflicts in include path
#include "../../mapgen.h"

#include <memory>
#include <unordered_set>

namespace editor
{

static void import_palette_data( Project &project, Palette &palette,
        const EID::Palette &source_id, PaletteImportReport &report )
{
    palette_id id( source_id.data );
    const mapgen_palette &source = *id;
    palette.imported_id = id;
    palette.imported = true;

    auto anc_data = calc_palette_ancestors(source);
    PaletteAncestorList list;
    for (const auto& anc : anc_data) {
        PaletteAncestorSwitch sw;
        sw.options = anc;
        list.list.push_back(sw);
    }
    palette.ancestors = list;

    std::unordered_set<map_key> all_keys;

    for( const auto &it : source.format_placings ) {
        all_keys.insert( it.first );
    }
    for( const map_key &key : all_keys ) {
        editor::Mapping mapping;
        {
            auto it = source.format_placings.find( key );
            if( it != source.format_placings.end() ) {
                for( const auto &piece_ptr : it->second ) {
                    const jmapgen_piece &piece = *piece_ptr;
                    std::unique_ptr<Piece> new_piece;
                    // TODO: this is inefficient O(n^2)
                    for( const PieceType &type : all_enum_values<PieceType>() ) {
                        new_piece = make_new_piece( type );
                        if( new_piece->try_import( piece, report ) ) {
                            break;
                        } else {
                            new_piece.reset();
                        }
                    }
                    report.num_total += 1;
                    if( !new_piece ) {
                        report.num_failed += 1;
                        new_piece = make_new_piece(PieceType::Unknown);
                    }
                    new_piece->uuid = project.uuid_generator();
                    mapping.pieces.emplace_back(std::move(new_piece));
                }
            }
        }
        editor::PaletteEntry entry = make_simple_entry( project, palette, std::move( mapping ) );
        entry.key = key;
        palette.entries.emplace_back( std::move( entry ) );
    }
}


void import_palette_data_and_report(State& state, Palette& destination, EID::Palette source)
{
    PaletteImportReport rep;
    import_palette_data(state.project(), destination, source, rep);

    bool is_ok = true;
    std::string error_text;
    if (rep.num_failed != 0) {
        is_ok = false;
        error_text += string_format(
            "%d out of %d mappings couldn't be resolved.\n\n",
            rep.num_failed, rep.num_total
        );
    }
    if (rep.num_values_folded) {
        is_ok = false;
        error_text += string_format(
            "%d mapgen_values have been collapsed to plain ids.\n\n",
            rep.num_values_folded
        );
    }

    if (!is_ok && state.ui->warn_on_import_issues) {
        std::string text = "Palette has been loaded with issues.\n\n" + error_text;
        state.control->show_warning_popup(text);
    }
}

void reimport_palette(State& state, Palette& p)
{
    p.entries.clear();
    p.ancestors.clear();
    import_palette_data_and_report(state, p, p.imported_id);
}

void quick_import_palette(State& state, EID::Palette p)
{
    Palette& new_palette = quick_create_palette(state);

    import_palette_data_and_report(state, new_palette, p);
    new_palette.name = p.data;
}

Palette& quick_create_palette(State& state)
{
    Project& project = state.project();
    UUID new_palette_uuid = project.uuid_generator();
    project.palettes.emplace_back();
    Palette& new_palette = project.palettes.back();
    new_palette.uuid = new_palette_uuid;
    return new_palette;
}

} // namespace editor
