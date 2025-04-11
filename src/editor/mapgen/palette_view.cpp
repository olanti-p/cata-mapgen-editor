#include "palette_view.h"

#include <algorithm>

#include "mapgen/piece_impl.h"
#include "common/map_key.h"

#include "project/project.h"
#include "state/state.h"
#include "state/ui_state.h"

namespace editor
{

const Piece* ViewEntry::get_first_piece_of_type(PieceType pt) const
{
    for( const ViewPiece &piece : pieces ) {
        if (piece.piece->get_type() == pt) {
            return piece.piece;
        }
    }
    return nullptr;
}

void ViewEntry::rebuild_sprite_cache(const Project& project) const
{
    sprite_cache_ter.reset();
    sprite_cache_furn.reset();

    {
        const Piece* ptr_raw = get_first_piece_of_type(PieceType::AltTerrain);
        const PieceAltTerrain* ptr = dynamic_cast<const PieceAltTerrain*>(ptr_raw);
        if (ptr) {
            auto list = ptr->list;
            if (!list.entries.empty() && !list.entries[0].val.is_null()) {
                sprite_cache_ter = SpriteRef(list.entries[0].val.data);
            }
        }
    }

    {
        const Piece* ptr_raw = get_first_piece_of_type(PieceType::AltFurniture);
        const PieceAltFurniture* ptr = dynamic_cast<const PieceAltFurniture*>(ptr_raw);
        if (ptr) {
            auto list = ptr->list;
            if (!list.entries.empty() && !list.entries[0].val.is_null()) {
                sprite_cache_furn = SpriteRef(list.entries[0].val.data);
            }
        }
    }

    sprite_cache_valid = true;
}

const ImVec4 &ViewPalette::color_from_uuid( const MapKey &uuid ) const
{
    /* TODO: palette colors
    const ViewEntry*entry = find_entry( uuid );
    if( entry ) {
        return entry->color;
    }
    */

    static ImVec4 default_color(0.0f, 0.0f, 0.0f, 1.0f);
    return default_color;
}

SpritePair ViewPalette::sprite_from_uuid( const MapKey &uuid ) const
{
    SpritePair ret;

    const ViewEntry*entry = find_entry( uuid );
    if( entry ) {
        if( !entry->sprite_cache_valid ) {
            entry->rebuild_sprite_cache(project);
        }
        if (entry->sprite_cache_ter) {
            ret.ter = &*entry->sprite_cache_ter;
        }
        if (entry->sprite_cache_furn) {
            ret.furn = &*entry->sprite_cache_furn;
        }
    }

    return ret;
}

ViewEntry *ViewPalette::find_entry( const MapKey&uuid )
{
    if (!uuid) {
        return nullptr;
    }
    if (entries.size() != entries_cache.size()) {
        rebuild_cache();
    }
    auto it = entries_cache.find(uuid);
    if (it == entries_cache.end()) {
        return nullptr;
    }
    if (entries[it->second].key != uuid) {
        rebuild_cache();
        auto it = entries_cache.find(uuid);
        if (it == entries_cache.end()) {
            return nullptr;
        }
    }
    return &entries[it->second];
}

const ViewEntry *ViewPalette::find_entry( const MapKey&uuid ) const
{
    if (!uuid) {
        return nullptr;
    }
    if (entries.size() != entries_cache.size()) {
        rebuild_cache();
    }
    auto it = entries_cache.find(uuid);
    if (it == entries_cache.end()) {
        return nullptr;
    }
    if (entries[it->second].key != uuid) {
        rebuild_cache();
        auto it = entries_cache.find(uuid);
        if (it == entries_cache.end()) {
            return nullptr;
        }
    }
    return &entries[it->second];
}

int ViewPalette::num_pieces_total() const
{
    size_t ret = 0;
    for (const ViewEntry& it : entries) {
        ret += it.pieces.size();
    }
    return ret;
}

void ViewPalette::rebuild_cache() const {
    entries_cache.clear();
    for (size_t i = 0; i < entries.size(); i++) {
        entries_cache[entries[i].key] = i;
        entries[i].rebuild_sprite_cache(project);
    }
}

void ViewPalette::invalidate_caches() const
{
    for (const ViewEntry& entry : entries) {
        entry.sprite_cache_valid = false;
    }
}

void ViewPalette::add_palette( Palette& pal)
{
    if (std::find(palettes.begin(), palettes.end(), &pal) != palettes.end()) {
        return;
    }
    palettes.emplace_back(&pal);

    for (const PaletteEntry& entry : pal.entries) {
        const MapKey& key = entry.key;
        ViewEntry* vm;
        if (entries_cache.count(key) == 0) {
            entries.emplace_back();
            vm = &*entries.rbegin();
            vm->key = key;
            entries_cache[key] = entries.size() - 1;
        }
        else {
            vm = &entries[entries_cache[key]];
        }
        for (const auto& piece : entry.pieces) {
            PieceType piece_type = piece->get_type();
            if (is_piece_exclusive(piece_type)) {
                auto new_end = std::remove_if(vm->pieces.begin(), vm->pieces.end(), [=](const ViewPiece& vp) {
                    return !vp.piece->constraint.has_value() && vp.piece->get_type() == piece_type;
                });
                vm->pieces.erase(new_end, vm->pieces.end());
            }
            vm->pieces.push_back(ViewPiece{ &pal, piece.get() });
        }
    }
}

void ViewPalette::add_palette_recursive(Palette& pal, ViewPaletteTreeState& vpts)
{
    std::vector<int>& selected_opts_palette = vpts.selected_opts[pal.uuid];
    selected_opts_palette.resize(pal.ancestors.list.size());
    for (size_t list_idx = 0; list_idx < pal.ancestors.list.size(); list_idx++) {
        const PaletteAncestorSwitch& ref = pal.ancestors.list[list_idx];
        int &selected_opt_this_list = selected_opts_palette[list_idx];
        if (selected_opt_this_list >= ref.options.size()) {
            selected_opt_this_list = ref.options.size() - 1;
        }
        const std::string& opt = ref.options[selected_opt_this_list];
        Palette* p = project.find_palette_by_string(opt);
        if (p) {
            add_palette_recursive(*p, vpts);
        }
    }
    add_palette(pal);
}

} // namespace editor
