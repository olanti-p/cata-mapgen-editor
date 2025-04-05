#ifndef CATA_SRC_EDITOR_PALETTE_H
#define CATA_SRC_EDITOR_PALETTE_H

#include "defs.h"

#include <vector>
#include <memory>

#include "imgui.h"
#include "mapgen_map_key.h"

#include "piece.h"
#include "common/uuid.h"
#include "common/sprite_ref.h"

struct ImDrawList;
struct ImVec4;
class JsonOut;
class JSON_IN;
template<typename T> struct enum_traits;
struct SpriteRef;

namespace editor
{
struct Mapgen;
struct Project;

struct Mapping {
    std::vector<std::unique_ptr<Piece>> pieces;

    Mapping() = default;
    Mapping( const Mapping &rhs );
    Mapping( Mapping && ) = default;
    ~Mapping() = default;

    Mapping &operator=( const Mapping &rhs );
    Mapping &operator=( Mapping && ) = default;

    void serialize( JsonOut &jsout ) const;
    void deserialize( const TextJsonValue &jsin );

    template<typename T>
    const T *get_first_piece_of_type() const {
        for( auto &piece : pieces ) {
            T *ptr = dynamic_cast<T *>( piece.get() );
            if( ptr ) {
                return ptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    T *get_first_piece_of_type() {
        const Mapping *this_c = this;
        return const_cast<T *>( this_c->get_first_piece_of_type<T>() );
    }

    bool has_piece_of_type( PieceType pt ) const;
};

struct PaletteEntry {
    map_key key;
    ImVec4 color;
    std::string name;
    Mapping mapping;

    mutable bool sprite_cache_valid = false;
    mutable std::optional<SpriteRef> sprite_cache;

    void build_sprite_cache() const;
    std::string display_name() const;

    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonValue& jsin);
};

struct Palette {
public:
    UUID uuid;
    bool imported = false;
    EID::Palette imported_id;
    std::string created_id;
    std::string name;
    std::optional<UUID> inherits_from;
    std::vector<PaletteEntry> entries;

    const std::string *display_key_from_uuid( const map_key &uuid ) const;
    const ImVec4 &color_from_uuid( const map_key &uuid ) const;
    const SpriteRef *sprite_from_uuid( const map_key &uuid ) const;

    PaletteEntry *find_entry( const map_key &uuid );
    const PaletteEntry *find_entry( const map_key &uuid ) const;

    std::string display_name() const;

    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonValue& jsin);

private:
    mutable std::unordered_map<map_key, size_t> entries_cache;
    void rebuild_cache() const;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_H
