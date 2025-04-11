#include "all_enum_values.h"
#include "mapgen/piece_type.h"
#include "palette_making.h"

#include "mapgen/palette.h"
#include "common/map_key.h"
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

std::unique_ptr<Piece> import_simple_piece(const jmapgen_piece& piece, PaletteImportReport& report)
{
    std::unique_ptr<Piece> new_piece;
    // TODO: optimize
    for (const PieceType& type : all_enum_values<PieceType>()) {
        new_piece = make_new_piece(type);
        if (new_piece->try_import(piece, report)) {
            break;
        }
        else {
            new_piece.reset();
        }
    }
    report.num_pieces_total += 1;

    if (!new_piece) {
        report.num_pieces_failed += 1;
        new_piece = make_new_piece(PieceType::Unknown);
    }
    return new_piece;
}

static std::unique_ptr<Piece> try_import_recursive(Project& project, const jmapgen_piece& piece, PaletteImportReport& report) {
    if (piece.is_constrained()) {
        const jmapgen_piece* inner = piece.get_constrained_inner();
        if (inner->is_constrained()) {
            // TODO: Can constrained pieces can be multiple levels deep?
            std::abort();
        }
        report.num_constrained++;
        std::unique_ptr<Piece> inner_imported = try_import_recursive(project, *inner, report);
        inner_imported->constraint = PieceConstraint();
        inner_imported->uuid = project.uuid_generator();
        return inner_imported;
    }
    else {
        std::unique_ptr<Piece> new_piece = import_simple_piece(piece, report);
        new_piece->uuid = project.uuid_generator();
        return new_piece;
    }
}

static void import_palette_data_internal( Project &project, Palette &palette,
    const mapgen_palette& source, PaletteImportReport &report )
{
    palette.imported = true;

    auto anc_data = calc_palette_ancestors(source);
    PaletteAncestorList list;
    for (const auto& anc : anc_data) {
        PaletteAncestorSwitch sw;
        sw.options = anc;
        list.list.push_back(sw);
    }
    palette.ancestors = list;

    std::unordered_set<MapKey> all_keys;

    for( const auto &it : source.format_placings ) {
        all_keys.insert( it.first );
    }
    for( const MapKey &key : all_keys ) {
        editor::PaletteEntry entry = make_simple_entry( project, palette, nullptr, nullptr );
        entry.key = key;
        {
            auto it = source.format_placings.find( key );
            if( it != source.format_placings.end() ) {
                for( const auto &piece_ptr : it->second ) {
                    std::unique_ptr<Piece> new_piece = try_import_recursive(project, *piece_ptr, report);
                    entry.pieces.emplace_back(std::move(new_piece));
                }
            }
        }
        palette.entries.emplace_back( std::move( entry ) );
        report.num_mappings++;
    }
}

static void import_palette_data(Project& project, Palette& palette,
    const EID::Palette& source_id, PaletteImportReport& report)
{
    palette_id id(source_id.data);
    const mapgen_palette& source = *id;
    palette.imported_id = id;

    import_palette_data_internal(project, palette, source, report);
}


void show_report(State& state, const PaletteImportReport& rep)
{
    bool is_ok = true;
    std::string error_text;
    if (rep.num_pieces_failed != 0) {
        is_ok = false;
        error_text += string_format(
            "%d out of %d pieces couldn't be resolved.\n\n",
            rep.num_pieces_failed, rep.num_pieces_total
        );
    }
    if (rep.num_values_folded) {
        is_ok = false;
        error_text += string_format(
            "%d mapgen_values have been collapsed to plain ids.\n\n",
            rep.num_values_folded
        );
    }

    if (!is_ok) {
        std::string text = "Palette has been loaded with issues.\n\n" + error_text;
        state.control->show_warning_popup(text);
    }
}


void import_palette_data_and_report(State& state, Palette& destination, EID::Palette source)
{
    PaletteImportReport rep;
    import_palette_data(state.project(), destination, source, rep);
    destination.import_report = rep;
    if (state.ui->warn_on_import_issues) {
        show_report(state, rep);
    }
}

void import_temp_palette_data_and_report(State& state, Palette& destination, const mapgen_palette& source)
{
    PaletteImportReport rep;
    import_palette_data_internal(state.project(), destination, source, rep);
    destination.import_report = rep;
    if (state.ui->warn_on_import_issues) {
        show_report(state, rep);
    }
}

void reimport_palette(State& state, Palette& p)
{
    p.entries.clear();
    p.ancestors.clear();

    EID::TempPalette tmp_id(p.imported_id.data);
    if (tmp_id.is_valid()) {
        const mapgen_palette& p_obj = get_temp_mapgen_palette(tmp_id.data);
        import_temp_palette_data_and_report(state, p, p_obj);
    }
    else {
        import_palette_data_and_report(state, p, p.imported_id);
    }
}

static void recursive_import_inner(State& state, Palette& p) {
    auto list_copy = p.ancestors.list;
    for (const auto& it : list_copy) {
        for (const auto& opt : it.options) {
            Palette* pal = state.project().find_palette_by_string(opt);
            if (pal) {
                recursive_import_inner(state, *pal);
            }
            else {
                EID::Palette p_id(opt);
                if (p_id.is_valid()) {
                    Palette& pal = quick_import_palette(state, p_id);
                    recursive_import_inner(state, pal);
                }
            }
        }
    }
}

void recursive_import_palette(State& state, Palette& p)
{
    recursive_import_inner(state, p);
}

Palette& quick_import_palette(State& state, EID::Palette p)
{
    Palette& new_palette = quick_create_palette(state);

    import_palette_data_and_report(state, new_palette, p);
    new_palette.name = p.data;
    return new_palette;
}

void quick_import_temp_palette(State& state, EID::TempPalette p)
{
    Palette& new_palette = quick_create_palette(state);

    const mapgen_palette &p_obj = get_temp_mapgen_palette(p.data);

    import_temp_palette_data_and_report(state, new_palette, p_obj);
    new_palette.imported_id = p.data;
    new_palette.name = new_palette.imported_id.data;
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
