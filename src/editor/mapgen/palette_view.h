#ifndef CATA_SRC_EDITOR_PALETTE_VIEW_H
#define CATA_SRC_EDITOR_PALETTE_VIEW_H

#include "defs.h"

#include <vector>
#include <memory>
#include <optional>

#include "imgui.h"
#include "common/map_key.h"
#include "palette_import_report.h"

#include "piece_type.h"
#include "common/uuid.h"
#include "common/sprite_ref.h"

struct ImVec4;
struct SpriteRef;

namespace editor
{
struct Palette;
struct Project;
struct Piece;
struct ViewPaletteTreeState;

struct ViewPiece {
    Palette* pal = nullptr;
    Piece* piece = nullptr;
};

struct ViewEntry {
    MapKey key;
    std::vector<ViewPiece> pieces;

    ViewEntry() = default;
    ViewEntry( const ViewEntry & ) = default;
    ViewEntry( ViewEntry && ) = default;
    ~ViewEntry() = default;

    ViewEntry &operator=( const ViewEntry & ) = default;
    ViewEntry &operator=( ViewEntry && ) = default;

    const Piece* get_first_piece_of_type(PieceType pt) const;

    template<typename T>
    const T* get_first_piece_of_type() const {
        for (auto& it : pieces) {
            T* ptr = dynamic_cast<T*>(it.piece);
            if (ptr) {
                return ptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    T* get_first_piece_of_type() {
        const ViewEntry* this_c = this;
        return const_cast<T*>(this_c->get_first_piece_of_type<T>());
    }

    void rebuild_sprite_cache( const Project& project ) const;

    mutable bool sprite_cache_valid = false;
    mutable std::optional<SpriteRef> sprite_cache_ter;
    mutable std::optional<SpriteRef> sprite_cache_furn;
};

struct ViewPaletteAncestorSwitch {
    std::vector<Palette*> options;
    int selected = 0;
};

struct ViewPaletteAncestorList {
    std::vector<ViewPaletteAncestorSwitch> list;
};

struct SpritePair {
    const SpriteRef* ter = nullptr;
    const SpriteRef* furn = nullptr;

    bool is_empty() const {
        return !ter && !furn;
    }
};

struct ViewPalette {
public:
    ViewPalette(Project& project_) : project(project_) {}

    Project& project;
    std::vector<ViewEntry> entries;

    const ImVec4 &color_from_uuid( const MapKey &uuid ) const;
    SpritePair sprite_from_uuid( const MapKey &uuid ) const;

    ViewEntry *find_entry( const MapKey &uuid );
    const ViewEntry *find_entry( const MapKey &uuid ) const;

    int num_pieces_total() const;

    void invalidate_caches() const;
    void add_palette(Palette& pal);
    void add_palette_recursive( Palette& pal, ViewPaletteTreeState& vpts);
    void finalize() {
        rebuild_cache();
    }

private:
    std::vector<Palette*> palettes;
    mutable std::unordered_map<MapKey, size_t> entries_cache;
    void rebuild_cache() const;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_VIEW_H
