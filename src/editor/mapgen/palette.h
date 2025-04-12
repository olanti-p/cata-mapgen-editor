#ifndef CATA_SRC_EDITOR_PALETTE_H
#define CATA_SRC_EDITOR_PALETTE_H

#include "defs.h"

#include <vector>
#include <memory>

#include <imgui/imgui.h>
#include "common/map_key.h"
#include "palette_import_report.h"

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

struct PaletteEntry {
    MapKey key;
    ImVec4 color;
    std::string name;
    std::vector<std::unique_ptr<Piece>> pieces;

    mutable bool sprite_cache_valid = false;
    mutable std::optional<SpriteRef> sprite_cache;

    PaletteEntry() = default;
    PaletteEntry(const PaletteEntry& rhs);
    PaletteEntry(PaletteEntry&&) = default;
    ~PaletteEntry() = default;

    PaletteEntry& operator=(const PaletteEntry& rhs);
    PaletteEntry& operator=(PaletteEntry&&) = default;

    void serialize(JsonOut& jsout) const;
    void deserialize(const TextJsonValue& jsin);

    Piece* find_piece(UUID uuid) {
        const PaletteEntry* this_c = this;
        return const_cast<Piece*>(this_c->find_piece(uuid));
    }
    const Piece* find_piece(UUID uuid) const;

    const Piece* get_first_piece_of_type(PieceType pt) const;

    template<typename T>
    const T* get_first_piece_of_type() const {
        for (auto& piece : pieces) {
            T* ptr = dynamic_cast<T*>(piece.get());
            if (ptr) {
                return ptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    T* get_first_piece_of_type() {
        const PaletteEntry* this_c = this;
        return const_cast<T*>(this_c->get_first_piece_of_type<T>());
    }

    bool has_piece_of_type(PieceType pt) const;
    void build_sprite_cache() const;
    std::string display_name() const;
};

struct PaletteAncestorSwitch {
    std::vector<std::string> options;
};

struct PaletteAncestorList {
    std::vector<PaletteAncestorSwitch> list;

    void clear() {
        list.clear();
    }
};

struct Palette {
public:
    UUID uuid;
    bool imported = false;
    bool standalone = true;
    bool temp_palette = false;
    EID::Palette imported_id;
    std::string created_id;
    std::string name;
    // Cached ancestors tree
    PaletteAncestorList ancestors;
    std::vector<PaletteEntry> entries;
    PaletteImportReport import_report;

    const ImVec4 &color_from_uuid( const MapKey &uuid ) const;
    const SpriteRef *sprite_from_uuid( const MapKey &uuid ) const;

    PaletteEntry *find_entry( const MapKey &uuid );
    const PaletteEntry *find_entry( const MapKey &uuid ) const;

    std::string display_name() const;
    int num_pieces_total() const;

    void serialize( JsonOut &jsout ) const;
    void deserialize(const TextJsonValue& jsin);

private:
    mutable std::unordered_map<MapKey, size_t> entries_cache;
    void rebuild_cache() const;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_H
