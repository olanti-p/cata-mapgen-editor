#include "palette.h"

#include "mapgen/piece_impl.h"
#include "common/map_key.h"

namespace editor
{

PaletteEntry::PaletteEntry( const PaletteEntry&rhs )
{
    *this = rhs;
}

PaletteEntry& PaletteEntry::operator=( const PaletteEntry&rhs )
{
    key = rhs.key;
    color = rhs.color;
    name = rhs.name;
    sprite_cache = rhs.sprite_cache;
    sprite_cache_valid = rhs.sprite_cache_valid;

    pieces.reserve( rhs.pieces.size() );
    for( const auto &piece : rhs.pieces ) {
        pieces.emplace_back( piece->clone() );
    }
    return *this;
}

bool PaletteEntry::has_piece_of_type( PieceType pt ) const
{
    for( const auto &piece : pieces ) {
        if( piece->get_type() == pt ) {
            return true;
        }
    }
    return false;
}

const Piece* PaletteEntry::find_piece(UUID uuid) const
{
    for (const auto& piece : pieces) {
        if (piece->uuid == uuid) {
            return piece.get();
        }
    }
    return nullptr;
}

const Piece* PaletteEntry::get_first_piece_of_type(PieceType pt) const
{
    for (const auto& piece : pieces) {
        if (piece->get_type() == pt) {
            return piece.get();
        }
    }
    return nullptr;
}

const std::string *Palette::display_key_from_uuid( const MapKey &uuid ) const
{
    const PaletteEntry *entry = find_entry( uuid );
    if( entry ) {
        return &entry->key.str;
    }

    return nullptr;
}

const ImVec4 &Palette::color_from_uuid( const MapKey &uuid ) const
{
    const PaletteEntry *entry = find_entry( uuid );
    if( entry ) {
        return entry->color;
    }

    static ImVec4 default_color = ImVec4();
    return default_color;
}

const SpriteRef *Palette::sprite_from_uuid( const MapKey &uuid ) const
{
    const PaletteEntry *entry = find_entry( uuid );
    if( entry ) {
        if( !entry->sprite_cache_valid ) {
            entry->build_sprite_cache();
        }
        if( !entry->sprite_cache ) {
            return nullptr;
        } else {
            return &*entry->sprite_cache;
        }
    }

    // Entry not found in palette
    return nullptr;
}

PaletteEntry *Palette::find_entry( const MapKey&uuid )
{
    if (uuid.str.empty()) {
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

const PaletteEntry *Palette::find_entry( const MapKey&uuid ) const
{
    if (uuid.str.empty()) {
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

std::string Palette::display_name() const
{
    if (!name.empty()) {
        return name;
    }
    else if (imported && !imported_id.data.empty()) {
        return imported_id.data;
    } else if (!imported && !created_id.empty()) {
        return created_id;
    } else {
        return string_format( "[uuid=%d]", uuid );
    }
}

int Palette::num_pieces_total() const
{
    size_t ret = 0;
    for (const PaletteEntry& it : entries) {
        ret += it.pieces.size();
    }
    return ret;
}

void Palette::rebuild_cache() const {
    entries_cache.clear();
    for (size_t i = 0; i < entries.size(); i++) {
        entries_cache[entries[i].key] = i;
    }
}

void PaletteEntry::build_sprite_cache() const
{
    sprite_cache.reset();

    // Try furniture tile
    {
        const PieceAltFurniture *ptr = get_first_piece_of_type<PieceAltFurniture>();
        if( ptr ) {
            auto list = ptr->list;
            if( !list.entries.empty() && !list.entries[0].val.is_null() ) {
                sprite_cache = SpriteRef( list.entries[0].val.data );
            }
        }
    }

    // Try terrain tile
    if( !sprite_cache ) {
        const PieceAltTerrain *ptr = get_first_piece_of_type<PieceAltTerrain>();
        if( ptr ) {
            auto list = ptr->list;
            if( !list.entries.empty() && !list.entries[0].val.is_null() ) {
                sprite_cache = SpriteRef( list.entries[0].val.data );
            }
        }
    }

    sprite_cache_valid = true;
}

std::string PaletteEntry::display_name() const
{
    return name;
}

} // namespace editor
