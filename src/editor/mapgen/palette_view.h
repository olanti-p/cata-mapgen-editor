#ifndef CATA_SRC_EDITOR_PALETTE_VIEW_H
#define CATA_SRC_EDITOR_PALETTE_VIEW_H

#include "defs.h"

#include <vector>
#include <memory>
#include <optional>

#include "imgui.h"
#include "mapgen_map_key.h"
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
    UUID palette;
    UUID id;
};

struct ViewMapping {
    map_key key;
    std::vector<ViewPiece> pieces;

    ViewMapping() = default;
    ViewMapping( const ViewMapping & ) = default;
    ViewMapping( ViewMapping && ) = default;
    ~ViewMapping() = default;

    ViewMapping &operator=( const ViewMapping & ) = default;
    ViewMapping &operator=( ViewMapping && ) = default;

    const Piece* get_first_piece_of_type(const Project& project, PieceType pt) const;
    void rebuild_sprite_cache( const Project& project ) const;

    mutable bool sprite_cache_valid = false;
    mutable std::optional<SpriteRef> sprite_cache_ter;
    mutable std::optional<SpriteRef> sprite_cache_furn;
};

struct ViewPaletteAncestorSwitch {
    std::vector<UUID> options;
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
    ViewPalette(const Project& project_) : project(project_) {}

    const Project& project;
    std::vector<ViewMapping> entries;

    const std::string *display_key_from_uuid( const map_key &uuid ) const;
    const ImVec4 &color_from_uuid( const map_key &uuid ) const;
    SpritePair sprite_from_uuid( const map_key &uuid ) const;

    ViewMapping *find_entry( const map_key &uuid );
    const ViewMapping *find_entry( const map_key &uuid ) const;

    int num_pieces_total() const;

    void invalidate_caches() const;
    void add_palette(const Palette& pal);
    void add_palette_recursive(const Palette& pal, ViewPaletteTreeState& vpts);
    void finalize() {
        rebuild_cache();
    }

private:
    std::vector<UUID> palettes;
    mutable std::unordered_map<map_key, size_t> entries_cache;
    void rebuild_cache() const;
};

} // namespace editor

#endif // CATA_SRC_EDITOR_PALETTE_VIEW_H
