#include "palette_making.h"

#include "common/color.h"
#include "mapgen/map_key_gen.h"
#include "mapgen/piece_impl.h"
#include "project/project.h"

namespace editor
{
map_key pick_available_key( const Palette &pal )
{
    MapKeyGenerator gen;
    for( const auto &it : pal.entries ) {
        gen.blacklist( it.key );
    }
    return gen();
}

PaletteEntry make_simple_entry( Project &project, Palette &palette, const EID::Ter* ter, const EID::Furn* furn)
{
    PaletteEntry ret;
    if (ter) {
        std::unique_ptr<PieceAltTerrain> piece = std::make_unique<PieceAltTerrain>();
        piece->init_new();
        piece->list.entries[0].weight = 1;
        piece->list.entries[0].val = *ter;
        ret.pieces.emplace_back(std::move(piece));
    }
    if (furn) {
        std::unique_ptr<PieceAltFurniture> piece = std::make_unique<PieceAltFurniture>();
        piece->init_new();
        piece->list.entries[0].weight = 1;
        piece->list.entries[0].val = *furn;
        ret.pieces.emplace_back(std::move(piece));
    }
    ret.key = pick_available_key(palette);
    ret.color = col_default_piece_color;
    return ret;
}

} // namespace editor
