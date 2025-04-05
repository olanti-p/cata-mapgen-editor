#include "all_enum_values.h"
#include "mapgen/piece_type.h"
#include "palette_making.h"

#include "mapgen/palette.h"
#include "mapgen_map_key.h"
#include "type_id.h"
#include "widget/editable_id.h"

// FIXME: conflicts in include path
#include "../../mapgen.h"

#include <memory>
#include <unordered_set>

namespace editor
{

PaletteImportReport import_palette_data( Project &project, Palette &palette,
        const EID::Palette &source_id )
{
    palette_id id( source_id.data );
    const mapgen_palette &source = *id;
    palette.name = id.str();
    palette.imported_id = id;
    palette.imported = true;

    PaletteImportReport report;

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
                    if( new_piece ) {
                        mapping.pieces.emplace_back( std::move( new_piece ) );
                    } else {
                        report.num_failed += 1;
                    }
                }
            }
        }
        editor::PaletteEntry entry = make_simple_entry( project, palette, std::move( mapping ) );
        entry.key = key;
        palette.entries.emplace_back( std::move( entry ) );
    }

    return report;
}

} // namespace editor
